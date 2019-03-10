#include "src/rcl/rclx/rclx_gason_util.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"

#include <optional>
#include <string>
#include <variant>

#include "3rdparty/gason/gason.h"

using namespace std::string_literals;

namespace rqdq {
namespace rclx {


std::optional<JsonValue> jv_find(const JsonValue& data, const std::string& key, int tag) {
	for (const auto& item : data) {
		if (key == item->key) {
			if (tag == -1 || item->value.getTag() == tag) {
				return item->value; } } }
	return std::nullopt; }


/**
 * {"$vec3": [nbr, nbr, nbr]} -> vec3{nbr, nbr, nbr}
 */
std::optional<rmlv::vec3> jv_decode_vec3(const JsonValue& data) {
	// a vec3 is
	if (data.getTag() == JSON_OBJECT) {
		// an object
		if (auto node = data.toNode(); node) {
			// with its first key/value pair
			if ("$vec3"s.compare(node->key) == 0) {
				// having a key of "$vec3"
				if (auto arr = node->value; arr.getTag() == JSON_ARRAY) {
					// and a value that is an array
					rmlv::vec3 out;
					int idx{ 0 };
					for (const auto& item : arr) {
						if (item->value.getTag() != JSON_NUMBER) {
							break; }
						out.arr[idx++] = float(item->value.toNumber());
						if (idx == 3) {
							// of exactly three numbers
							return out; } } } } } }
	return {}; }


std::variant<std::monostate, std::string, rmlv::vec3> jv_decode_ref_or_vec3(const JsonValue& data) {
	switch (data.getTag()) {
	case JSON_STRING:
		return data.toString();
	case JSON_OBJECT:
		if (auto value = jv_decode_vec3(data)) {
			return *value; } }
	return {}; }

}  // namespace rclx
}  // namespace rqdq
