#include "src/rgl/rglv/rglv_gpu.hxx"

#include <fmt/format.h>

#define UNUSED(expr) do { (void)(expr); } while (0)

namespace rqdq {
namespace rglv {

/**
 * global toggle for double-buffer operation of _all_ gpu
 * instances.
 *
 * may only be updated when all gpu instances are idle.
 */
bool doubleBuffer{true};

GPU::GPU(int concurrency, std::string guid) :
	concurrency_(concurrency),
	guid_(guid),
	color0Buf_(kTileColorSizeInBytes * concurrency),
	depthBuf_(kTileDepthSizeInBytes * concurrency),
	threadStats_(concurrency)
	{}

void GPU::Install(int programId, uint32_t stateKey, BinProgramPtrs ptrs) {
	uint32_t x = (programId << 24) | stateKey;
	if (auto item = binDispatch_.find(x); item != binDispatch_.end()) {
		std::cerr << "warning: duplicate GPU vertex program pid(" << programId << ") state(" << stateKey << ")\n"; }
	// std::cerr << "installed BinProgram " << x << "\n";
	binDispatch_[x] = ptrs; }


void GPU::Install(int programId, uint32_t stateKey, DrawProgramPtrs ptrs) {
	uint32_t x = (programId << 24) | stateKey;
	if (auto item = drawDispatch_.find(x); item != drawDispatch_.end()) {
		std::cerr << "warning: duplicate GPU fragment program pid(" << programId << ") state(" << stateKey << ")\n"; }
	// std::cerr << "installed DrawProgram " << x << "\n";
	drawDispatch_[x] = ptrs; }


void GPU::Install(int programId, uint32_t stateKey, BltProgramPtrs ptrs) {
	uint32_t x = (programId << 24) | stateKey;
	if (auto item = bltDispatch_.find(x); item != bltDispatch_.end()) {
		std::cerr << "warning: duplicate GPU blt program pid(" << programId << ") state(" << stateKey << ")\n"; }
	// std::cerr << "installed BltProgram " << x << "\n";
	bltDispatch_[x] = ptrs; }


void GPU::Reset(rmlv::ivec2 newBufferDimensionsInPixels, rmlv::ivec2 newTileDimensionsInBlocks) {
	SetSize(newBufferDimensionsInPixels, newTileDimensionsInBlocks);
	for (int t=0, siz=static_cast<int>(tilesHead_.size()); t<siz; ++t) {
		tilesHead_[t] = WriteRange(t).first; }
	clippedVertexBuffer0_.clear();
	stats0_ = GPUStats{};
	IC().Reset(); }


void GPU::SetSize(rmlv::ivec2 newBufferDimensionsInPixels, rmlv::ivec2 newTileDimensionsInBlocks) {
	if (newBufferDimensionsInPixels != bufferDimensionsInPixels_ ||
		newTileDimensionsInBlocks != tileDimensionsInBlocks_) {
		bufferDimensionsInPixels_ = newBufferDimensionsInPixels;
		tileDimensionsInBlocks_ = newTileDimensionsInBlocks;
		Retile(); }}


void GPU::Retile() {
	using rmlv::ivec2;
	using std::min, std::max;
	tileDimensionsInPixels_ = tileDimensionsInBlocks_ * kBlockDimensionsInPixels;
	bufferDimensionsInTiles_ = (bufferDimensionsInPixels_ + (tileDimensionsInPixels_ - ivec2{1, 1})) / tileDimensionsInPixels_;

	for (int ti=0; ti<concurrency_; ++ti) {
		threadStats_[ti].Reset(); }

	auto areaInTiles = Area(bufferDimensionsInTiles_);

	tilesMem0_.resize(kMaxSizeInBytes * areaInTiles);
	tilesMem1_.resize(kMaxSizeInBytes * areaInTiles);
	tilesHead_.clear();
	tilesHead_.resize(areaInTiles, nullptr);
	tilesMark_.clear();
	tilesMark_.resize(areaInTiles, nullptr);

	for (int ti{0}; ti<areaInTiles; ++ti) {
		tilesHead_[ti] = WriteRange(ti).first;
		tilesMem1_[ti * kMaxSizeInBytes] = CMD_EOF; }}


void GPU::RunImpl(rclmt::jobsys::Job* job) {
	namespace jobsys = rclmt::jobsys;
	auto finalizeJob = Finalize();
	if (job != nullptr) {
		jobsys::move_links(job, finalizeJob); }

	if (!doubleBuffer) {
		BinImpl();
		SwapBuffers(); }

	// sort tiles by their estimated cost before queueing
	tileStats_.clear();
	for (int t=0, siz=static_cast<int>(tilesHead_.size()); t<siz; ++t) {
		tileStats_.push_back({ t, RenderCost(t) }); }
	rclr::sort(tileStats_, [](auto a, auto b) { return a.cost > b.cost; });
	// std::rotate(tileStats_.begin(), tileStats_.begin() + 1, tileStats_.end())

	std::vector<rclmt::jobsys::Job*> allJobs;
	for (const auto& item : tileStats_) {
		int bi = item.tileId;
		allJobs.push_back(Draw(finalizeJob, bi)); }
	if (doubleBuffer) {
		allJobs.push_back(Bin(finalizeJob)); }

	for (auto& j : allJobs) {
		jobsys::run(j); }
	jobsys::run(finalizeJob); }


void GPU::BinImpl() {
	using fmt::printf;
	// printf("----- BEGIN BINNING -----\n");

	binState = nullptr;
	binUniforms = nullptr;
	auto& cs = IC().commands_;
	cs.appendByte(CMD_EOF);
	int totalCommands = 0;
	bool done{false};
	while (!done) {
		totalCommands += 1;
		auto cmd = cs.consumeByte();
		// printf("cmd %02x %d:", cmd, cmd);
		switch (cmd) {
		case CMD_EOF:
			for (auto& h : tilesHead_) {
				AppendByte(&h, cmd); }
			done = true;
			break;
		case CMD_STATE:
			binState = static_cast<GLState*>(cs.consumePtr());
			binUniforms = IC().GetUniformBufferAddr(binState->uniformsOfs);
			// printf(" state is now at %p\n", binState);
			for (auto& h : tilesHead_) {
				AppendByte(&h, cmd);
				AppendPtr(&h, binState);
				AppendPtr(&h, binUniforms); }
			break;
		case CMD_CLEAR: {
			auto arg = cs.consumeByte();
			// printf(" clear with color ");
			// std::cout << color << std::endl;
			for (auto& h : tilesHead_) {
				AppendByte(&h, cmd);
				AppendByte(&h, arg); }}
			break;
		case CMD_STORE_COLOR_HALF_LINEAR_FP: {
			auto ptr = cs.consumePtr();
			// printf(" store halfsize colorbuffer @ %p\n", ptr);
			for (auto& h : tilesHead_) {
				AppendByte(&h, cmd);
				AppendPtr(&h, ptr); }}
			break;
		case CMD_STORE_COLOR_FULL_LINEAR_FP: {
			auto ptr = cs.consumePtr();
			// printf(" store unswizzled colorbuffer @ %p\n", ptr);
			for (auto& h : tilesHead_) {
				AppendByte(&h, cmd);
				AppendPtr(&h, ptr); }}
			break;
		case CMD_STORE_COLOR_FULL_QUADS_FP: {
			auto ptr = cs.consumePtr();
			// printf(" store swizzled colorbuffer @ %p\n", ptr);
			for (auto& h : tilesHead_) {
				AppendByte(&h, cmd);
				AppendPtr(&h, ptr); }}
			break;
		case CMD_STORE_COLOR_FULL_LINEAR_TC: {
			auto enableGamma = cs.consumeByte();
			auto ptr = cs.consumePtr();
			// printf(" store truecolor @ %p, gamma corrected? %s\n", ptr, (enableGamma?"Yes":"No"));
			for (auto& h : tilesHead_) {
				AppendByte(&h, cmd);
				AppendByte(&h, enableGamma);
				AppendPtr(&h, ptr); }}
			break;
		case CMD_STORE_DEPTH_FULL_LINEAR_FP: {
			auto ptr = cs.consumePtr();
			for (auto& h : tilesHead_) {
				AppendByte(&h, cmd);
				AppendPtr(&h, ptr); }}
			break;
		case CMD_DRAW_ARRAYS: {
			//auto flags = cs.consumeByte();
			//assert(flags == 0x14);  // videocore: 16-bit indices, triangles
			auto count = cs.consumeInt();
			if (auto found = binDispatch_.find(binState->VertexStateKey());  found != binDispatch_.end()) {
				auto ptrs = found->second;
				((*this).*(ptrs.DrawArrays1))(count); }
			else {
				binState->Dump();
				fmt::print(stderr, "BIN:DRAW_ARRAYS no dispatch entry for 0x{:x}\n", binState->VertexStateKey());
				std::exit(1); }}
			break;
		case CMD_DRAW_ARRAYS_INSTANCED: {
			//auto flags = cs.consumeByte();
			//assert(flags == 0x14);  // videocore: 16-bit indices, triangles
			auto count = cs.consumeInt();
			auto instanceCnt = cs.consumeInt();
			if (auto found = binDispatch_.find(binState->VertexStateKey());  found != binDispatch_.end()) {
				auto ptrs = found->second;
				((*this).*(ptrs.DrawArraysN))(count, instanceCnt); }
			else {
				fmt::print(stderr, "BIN:DRAW_ARRAYS_INSTANCED no dispatch entry for 0x{:x}\n", binState->VertexStateKey());
				std::exit(1); }}
			break;
		case CMD_DRAW_ELEMENTS: {
			auto flags = cs.consumeByte();
			assert(flags == 0x14);  // videocore: 16-bit indices, triangles
			UNUSED(flags);
			auto hint = cs.consumeByte();
			auto count = cs.consumeInt();
			auto indices = static_cast<uint16_t*>(cs.consumePtr());
			if (auto found = binDispatch_.find(binState->VertexStateKey());  found != binDispatch_.end()) {
				auto ptrs = found->second;
				((*this).*(ptrs.DrawElements1))(count, indices, hint); }
			else {
				fmt::print(stderr, "BIN:DRAW_ELEMENTS no dispatch entry for 0x{:x}\n", binState->VertexStateKey());
				std::exit(1); }}
			break;
		case CMD_DRAW_ELEMENTS_INSTANCED: {
			auto flags = cs.consumeByte();
			assert(flags == 0x14);  // videocore: 16-bit indices, triangles
			UNUSED(flags);
			auto count = cs.consumeInt();
			auto indices = static_cast<uint16_t*>(cs.consumePtr());
			auto instanceCnt = cs.consumeInt();
			if (auto found = binDispatch_.find(binState->VertexStateKey());  found != binDispatch_.end()) {
				auto ptrs = found->second;
				((*this).*(ptrs.DrawElementsN))(count, indices, instanceCnt); }
			else {
				fmt::print(stderr, "BIN:DRAW_ELEMENTS_INSTANCED no dispatch entry for 0x{:x}\n", binState->VertexStateKey());
				std::exit(1); }}
			break; }}

	// stats...
	int total_ = 0;
	int min_ = 0x7fffffff;
	int max_ = 0;
	for (int t = 0, siz = static_cast<int>(tilesHead_.size()); t < siz; ++t) {
		const int thisTileBytes = static_cast<int>(tilesHead_[t] - WriteRange(t).first);
		total_ += thisTileBytes;
		min_ = std::min(min_, thisTileBytes);
		max_ = std::max(max_, thisTileBytes); }
	stats0_.totalTiles = static_cast<int>(tilesHead_.size());
	stats0_.totalCommands = totalCommands;
	// stats0_.totalStates = IC().d_si;
	stats0_.totalCommandBytes = cs.size();
	stats0_.totalTileCommandBytes = total_;
	stats0_.minTileCommandBytes = min_;
	stats0_.maxTileCommandBytes = max_; }


void GPU::DrawImpl(const unsigned tid, const int tileIdx) {
	const auto rect = TileRect(tileIdx);
	const auto tileOrigin = rect.topLeft;
	threadStats_[tid].tiles.emplace_back(tileIdx);

	const auto color0BufPtr = static_cast<void*>(color0Buf_.data() + kTileColorSizeInBytes*rclmt::jobsys::threadId);
	const auto depthBufPtr  = static_cast<void*>(depthBuf_.data()  + kTileDepthSizeInBytes*rclmt::jobsys::threadId);

	FastPackedReader cs{ ReadRange(tileIdx).first };
	int cmdCnt = 0;

	const GLState* stateptr{nullptr};
	const void* uniformptr{nullptr};
	void* eColor0{nullptr};
	void* eDepth{nullptr};

	bool done{false};
	while (!done) {
		++cmdCnt;
		auto cmd = cs.ConsumeByte();
		// fmt::printf("tile(%d) cmd(%02x, %d)\n", tileIdx, cmd, cmd);
		switch (cmd) {
		case CMD_EOF:
			done = true;
			break;
		case CMD_STATE: {
			stateptr = static_cast<const GLState*>(cs.ConsumePtr());
			uniformptr = cs.ConsumePtr();
			eColor0 = nullptr;
			eDepth = nullptr;
			if (stateptr->color0AttachmentType == RB_COLOR_DEPTH) {
				eColor0 = color0BufPtr; }
			else if (stateptr->color0AttachmentType == RB_RGBAF32) {
				eColor0 = color0BufPtr; }
			else if (stateptr->color0AttachmentType == RB_RGBF32) {
				eColor0 = color0BufPtr; }
			else {
				fmt::print(stderr, "RDR:STATE unknown color0 Renderbuffer type 0x{:x}\n", stateptr->color0AttachmentType);
				std::exit(1); }
			if (stateptr->depthAttachmentType == RB_COLOR_DEPTH) {
				assert(stateptr->color0AttachmentType == RB_COLOR_DEPTH);
				eDepth = color0BufPtr; }
			else if (stateptr->depthAttachmentType == RB_F32) {
				eDepth = depthBufPtr; }
			else {
				fmt::print(stderr, "RDR:STATE unknown depth Renderbuffer type 0x{:x}\n", stateptr->depthAttachmentType);
				std::exit(1); }}
			break;
		case CMD_CLEAR: {
			int bits = cs.ConsumeByte();
			if ((bits & GL_STENCIL_BUFFER_BIT) != 0) {
				std::cerr << "CLEAR on STENCIL not implemented\n";
				std::exit(1); }
			if (stateptr->color0AttachmentType == RB_COLOR_DEPTH) {
				if (bits != (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)) {
					std::cerr << "must clear color and depth when using RB_COLOR_DEPTH\n";
					std::exit(1); }
				const auto fillValue = rmlv::vec4{ stateptr->clearColor.xyz(), stateptr->clearDepth };
				rglr::QFloat4Canvas cc{ tileDimensionsInPixels_.x, tileDimensionsInPixels_.y, static_cast<rmlv::qfloat4*>(color0BufPtr), 64 };
				Fill(cc, fillValue); }
			else {
				if ((bits&GL_COLOR_BUFFER_BIT) != 0) {
					if (stateptr->color0AttachmentType == RB_RGBF32) {
						const auto fillValue = stateptr->clearColor.xyz();
						rglr::QFloat3Canvas cc{ tileDimensionsInPixels_.x, tileDimensionsInPixels_.y, static_cast<rmlv::qfloat3*>(color0BufPtr), 64 };
						Fill(cc, fillValue); }
					else if (stateptr->color0AttachmentType == RB_RGBAF32) {
						const auto fillValue = stateptr->clearColor;
						rglr::QFloat4Canvas cc{ tileDimensionsInPixels_.x, tileDimensionsInPixels_.y, static_cast<rmlv::qfloat4*>(color0BufPtr), 64 };
						Fill(cc, fillValue); }
					else {
						std::cerr << "can't clear color buffer of type RB " << stateptr->color0AttachmentType << "\n";
						std::exit(1); }}
				if ((bits & GL_DEPTH_BUFFER_BIT) != 0) {
					if (stateptr->depthAttachmentType == RB_F32) {
						const auto fillValue = stateptr->clearDepth;
						rglr::QFloatCanvas cc{ tileDimensionsInPixels_.x, tileDimensionsInPixels_.y, static_cast<rmlv::qfloat*>(depthBufPtr), 64 };
						Fill(cc, fillValue); }
					else {
						std::cerr << "can't clear depth buffer of type RB " << stateptr->depthAttachmentType << "\n";
						std::exit(1); }}}}
			break;
		case CMD_STORE_COLOR_HALF_LINEAR_FP: {
			if (stateptr->color0AttachmentType == RB_COLOR_DEPTH || stateptr->color0AttachmentType == RB_RGBAF32) {
				rglr::QFloat4Canvas cc{ tileDimensionsInPixels_.x, tileDimensionsInPixels_.y, static_cast<rmlv::qfloat4*>(color0BufPtr), 64 };
				auto dst = static_cast<rglr::FloatingPointCanvas*>(cs.ConsumePtr());
				Downsample(cc, *dst, rect); }
			else if (stateptr->color0AttachmentType == RB_RGBF32) {
				rglr::QFloat3Canvas cc{ tileDimensionsInPixels_.x, tileDimensionsInPixels_.y, static_cast<rmlv::qfloat3*>(color0BufPtr), 64 };
				auto dst = static_cast<rglr::FloatingPointCanvas*>(cs.ConsumePtr());
				Downsample(cc, *dst, rect); }
			else {
				std::cerr << "STORE_COLOR_HALF_LINEAR_FP not implemented for attachment type " << stateptr->color0AttachmentType << "\n";
				std::exit(1); }}
			break;
		case CMD_STORE_COLOR_FULL_LINEAR_FP: {
			if (stateptr->color0AttachmentType == RB_COLOR_DEPTH || stateptr->color0AttachmentType == RB_RGBAF32) {
				rglr::QFloat4Canvas cc{ tileDimensionsInPixels_.x, tileDimensionsInPixels_.y, static_cast<rmlv::qfloat4*>(color0BufPtr), 64 };
				auto dst = static_cast<rglr::FloatingPointCanvas*>(cs.ConsumePtr());
				Copy(cc, *dst, rect); }
			else if (stateptr->color0AttachmentType == RB_RGBF32) {
				rglr::QFloat3Canvas cc{ tileDimensionsInPixels_.x, tileDimensionsInPixels_.y, static_cast<rmlv::qfloat3*>(color0BufPtr), 64 };
				auto dst = static_cast<rglr::FloatingPointCanvas*>(cs.ConsumePtr());
				Copy(cc, *dst, rect); }
			else {
				std::cerr << "CMD_STORE_COLOR_HALF_LINEAR_FP not implemented for attachment type " << stateptr->color0AttachmentType << "\n";
				std::exit(1); }}
			break;
		case CMD_STORE_COLOR_FULL_QUADS_FP: {
			if (stateptr->color0AttachmentType == RB_COLOR_DEPTH || stateptr->color0AttachmentType == RB_RGBAF32) {
				rglr::QFloat4Canvas cc{ tileDimensionsInPixels_.x, tileDimensionsInPixels_.y, static_cast<rmlv::qfloat4*>(color0BufPtr), 64 };
				auto dst = static_cast<rglr::QFloat4Canvas*>(cs.ConsumePtr());
				Copy(cc, *dst, rect); }
			else if (stateptr->color0AttachmentType == RB_RGBF32) {
				rglr::QFloat3Canvas cc{ tileDimensionsInPixels_.x, tileDimensionsInPixels_.y, static_cast<rmlv::qfloat3*>(color0BufPtr), 64 };
				auto dst = static_cast<rglr::QFloat4Canvas*>(cs.ConsumePtr());
				Copy(cc, *dst, rect); }
			else {
				std::cerr << "CMD_STORE_COLOR_HALF_LINEAR_FP not implemented for attachment type " << stateptr->color0AttachmentType << "\n";
				std::exit(1); }}
			break;
		case CMD_STORE_COLOR_FULL_LINEAR_TC: {
			auto enableGamma = cs.ConsumeByte();
			auto& outcanvas = *static_cast<rglr::TrueColorCanvas*>(cs.ConsumePtr());
			if (auto found = bltDispatch_.find(stateptr->BltStateKey());  found != bltDispatch_.end()) {
				auto ptrs = found->second;
				((*this).*(ptrs.StoreTrueColor))(*stateptr, uniformptr, rect, enableGamma, outcanvas); }
			else {
				stateptr->Dump();
				std::cerr << "no dispatch entry for " << std::hex << stateptr->BltStateKey() << std::dec << " found for CMD_STORE_COLOR_FULL_LINEAR_TC\n";
				std::exit(1); }}
			// XXX draw cpu assignment indicators draw_border(rect, cpu_colors[tid], canvas);
			break;
		case CMD_STORE_DEPTH_FULL_LINEAR_FP: {
			auto p = cs.ConsumePtr();
			if (stateptr->depthAttachmentType == RB_F32) {
				rglr::QFloatCanvas src{ tileDimensionsInPixels_.x, tileDimensionsInPixels_.y, static_cast<rmlv::qfloat*>(depthBufPtr), 64 };
				auto dst = rglr::FloatCanvas(bufferDimensionsInPixels_.x, bufferDimensionsInPixels_.y, p, bufferDimensionsInPixels_.x);
				Copy(src, dst, rect); }
			else {
				std::cerr << "STORE_DEPTH_FULL_LINEAR_FP not implemented for attachment type " << stateptr->depthAttachmentType << "\n";
				std::exit(1); }}
			break;
		case CMD_CLIPPED_TRI: {
			if (auto found = drawDispatch_.find(stateptr->FragmentStateKey());  found != drawDispatch_.end()) {
				auto ptrs = found->second;
				((*this).*(ptrs.DrawClipped))(eColor0, eDepth, *stateptr, uniformptr, rect, tileOrigin, tileIdx, cs); }
			else {
				stateptr->Dump();
				std::cerr << "no dispatch entry for " << std::hex << stateptr->FragmentStateKey() << std::dec << " found for CMD_CLIPPED_TRI\n";
				std::exit(1); }}
			break;
		case CMD_DRAW_INLINE: {
			if (auto found = drawDispatch_.find(stateptr->FragmentStateKey());  found != drawDispatch_.end()) {
				auto ptrs = found->second;
				((*this).*(ptrs.DrawElements1))(eColor0, eDepth, *stateptr, uniformptr, rect, tileOrigin, tileIdx, cs); }
			else {
				stateptr->Dump();
				std::cerr << "no dispatch entry for " << std::hex << stateptr->FragmentStateKey() << std::dec << " found for CMD_DRAW_INLINE\n";
				std::exit(1); }}
			break;
		case CMD_DRAW_INLINE_INSTANCED: {
			if (auto found = drawDispatch_.find(stateptr->FragmentStateKey());  found != drawDispatch_.end()) {
				auto ptrs = found->second;
				((*this).*(ptrs.DrawElementsN))(eColor0, eDepth, *stateptr, uniformptr, rect, tileOrigin, tileIdx, cs); }
			else {
				stateptr->Dump();
				std::cerr << "no dispatch entry for " << std::hex << stateptr->FragmentStateKey() << std::dec << " found for CMD_DRAW_INLINE_INSTANCED\n";
				std::exit(1); }}
			break; }}}

}  // namespace rglv
}  // namespace rqdq
