#include <pmt_converter/pmt_legacy_codec.h>

#include <stdexcept>
#include <cstring>
#include <cmath>
#include <string>
#include <limits>
#include <variant>
#include <iostream>
#include <algorithm>
#include <vector>
#include <cstdint>
#include <bit>        // For std::endian and std::bit_cast
#include <complex>    // For std::complex
#include <cstring>    // For std::memcpy>

using namespace pmtv;

namespace legacy_pmt {

enum class legacy_tag : uint8_t {
    LEGACY_PMT_TRUE = 0x00,
    LEGACY_PMT_FALSE = 0x01,
    LEGACY_PMT_SYMBOL = 0x02,
    LEGACY_PMT_INT32 = 0x03,
    LEGACY_PMT_DOUBLE = 0x04,
    LEGACY_PMT_COMPLEX = 0x05,
    LEGACY_PMT_NULL = 0x06,
    LEGACY_PMT_PAIR = 0x07,
    LEGACY_PMT_VECTOR = 0x08,
    LEGACY_PMT_DICT = 0x09,    
    LEGACY_PMT_UNIFORM_VECTOR = 0x0A,
    LEGACY_PMT_UINT64 = 0x0B,
    LEGACY_PMT_TUPLE = 0x0C,
    LEGACY_PMT_INT64 = 0x0D
};

enum class legacy_uniform_type : uint8_t {
    U8 = 0x00,
    S8 = 0x01,
    U16 = 0x02,
    S16 = 0x03,
    U32 = 0x04,
    S32 = 0x05,
    U64 = 0x06,
    S64 = 0x07,
    F32 = 0x08,
    F64 = 0x09,    
    C32 = 0x0A,
    C64 = 0x0B,
    UNKNOWN = 0xFF
};

static void write_u8(std::vector<uint8_t>& out, uint8_t v) {
    out.push_back(v);
}

static void write_u16(std::vector<uint8_t>& out, uint32_t v) {
    for (int i = 1; i >= 0; --i)
        out.push_back((v >> (i * 8)) & 0xFF);
}

static void write_u32(std::vector<uint8_t>& out, uint32_t v) {
    for (int i = 3; i >= 0; --i)
        out.push_back((v >> (i * 8)) & 0xFF);
}

static void write_u64(std::vector<uint8_t>& out, uint64_t v) {
    for (int i = 7; i >= 0; --i)
        out.push_back((v >> (i * 8)) & 0xFF);
}

static void write_double(std::vector<uint8_t>& out, double d) {
    uint64_t bits;
    std::memcpy(&bits, &d, sizeof(double));
    write_u64(out, bits);
}

static uint64_t read_u32(const uint8_t*& data) {
    uint64_t result = 0;
    for (int i = 0; i < 4; ++i) {
        result = (result << 8) | data[i];
    }
    data += 4;
    return result;
}

static uint64_t read_u64(const uint8_t*& data) {
    uint64_t result = 0;
    for (int i = 0; i < 8; ++i) {
        result = (result << 8) | data[i];
    }
    data += 8;
    return result;
}

static double read_double(const uint8_t*& data) {
    uint64_t bits = read_u64(data);
    double d;
    std::memcpy(&d, &bits, sizeof(double));
    return d;
}

std::vector<uint8_t> serialize_integral(const pmtv::pmt& obj) {
    return std::visit([](const auto& val) -> std::vector<uint8_t> {
        using T = std::decay_t<decltype(val)>;
        std::vector<uint8_t> out;

        if constexpr (std::is_same_v<T, int32_t>) {
            write_u8(out, static_cast<uint8_t>(legacy_tag::LEGACY_PMT_INT32));
            write_u32(out, static_cast<uint32_t>(val));
        } else if constexpr (std::is_same_v<T, int64_t>) {
            write_u8(out, static_cast<uint8_t>(legacy_tag::LEGACY_PMT_INT64));
            write_u64(out, static_cast<uint64_t>(val));
        } else {
            throw std::runtime_error("Unsupported Integral PMT type for serialization");
        }

        return out;
    }, obj);

}

std::vector<uint8_t> serialize_real(const pmtv::pmt& obj) {
    return std::visit([](const auto& val) -> std::vector<uint8_t> {
        using T = std::decay_t<decltype(val)>;
        std::vector<uint8_t> out;

        write_u8(out, static_cast<uint8_t>(legacy_tag::LEGACY_PMT_DOUBLE));
        if constexpr (std::is_same_v<T, float>) {
            write_double(out, static_cast<double>(val));
        } else if constexpr (std::is_same_v<T, double>) {
            write_double(out, val);
        } else {
            throw std::runtime_error("Unsupported PMT type for serialization");
        }

        return out;
    }, obj);

}

std::vector<uint8_t> serialize_string(const std::string& str) {
    std::vector<uint8_t> out;

    write_u8(out, static_cast<uint8_t>(legacy_tag::LEGACY_PMT_SYMBOL));
    write_u16(out, static_cast<uint32_t>(str.size()));
    out.insert(out.end(), str.begin(), str.end());    

    return out;
}

template <typename T>
constexpr legacy_uniform_type legacy_uniform_type_for() {
    if constexpr (std::is_same_v<T, uint8_t>) return legacy_uniform_type::U8;
    else if constexpr (std::is_same_v<T, int8_t>) return legacy_uniform_type::S8;
    else if constexpr (std::is_same_v<T, uint16_t>) return legacy_uniform_type::U16;
    else if constexpr (std::is_same_v<T, int16_t>) return legacy_uniform_type::S16;
    else if constexpr (std::is_same_v<T, uint32_t>) return legacy_uniform_type::U32;
    else if constexpr (std::is_same_v<T, int32_t>) return legacy_uniform_type::S32;
    else if constexpr (std::is_same_v<T, uint64_t>) return legacy_uniform_type::U64;
    else if constexpr (std::is_same_v<T, int64_t>) return legacy_uniform_type::S64;
    else if constexpr (std::is_same_v<T, float>) return legacy_uniform_type::F32;
    else if constexpr (std::is_same_v<T, double>) return legacy_uniform_type::F64;
    else if constexpr (std::is_same_v<T, std::complex<float>>) return legacy_uniform_type::C32;
    else if constexpr (std::is_same_v<T, std::complex<double>>) return legacy_uniform_type::C64;
    else return legacy_uniform_type::UNKNOWN;
}


// Helper function to swap bytes if necessary
template <typename T>
requires std::is_integral_v<T> && (!std::is_same_v<T, bool>) // Ensure it's an integral type, but not bool
T to_big_endian_integral(T value) {
    if constexpr (std::endian::native == std::endian::little) {
        if constexpr (sizeof(T) == 1) {
            return value; // Single byte, no swap needed
        } else if constexpr (sizeof(T) == 2) {
            return static_cast<T>(
                ((static_cast<uint16_t>(value) & 0xFF00) >> 8) |
                ((static_cast<uint16_t>(value) & 0x00FF) << 8)
            );
        } else if constexpr (sizeof(T) == 4) {
            return static_cast<T>(
                ((static_cast<uint32_t>(value) & 0xFF000000) >> 24) |
                ((static_cast<uint32_t>(value) & 0x00FF0000) >> 8) |
                ((static_cast<uint32_t>(value) & 0x0000FF00) << 8) |
                ((static_cast<uint32_t>(value) & 0x000000FF) << 24)
            );
        } else if constexpr (sizeof(T) == 8) {
            uint64_t u64_val = static_cast<uint64_t>(value);
            return static_cast<T>(
                ((u64_val & 0xFF00000000000000ULL) >> 56) |
                ((u64_val & 0x00FF000000000000ULL) >> 40) |
                ((u64_val & 0x0000FF0000000000ULL) >> 24) |
                ((u64_val & 0x000000FF00000000ULL) >> 8)  |
                ((u64_val & 0x00000000FF000000ULL) << 8)  |
                ((u64_val & 0x0000000000FF0000ULL) << 24) |
                ((u64_val & 0x000000000000FF00ULL) << 40) |
                ((u64_val & 0x00000000000000FFULL) << 56)
            );
        } else {
            // Generic byte swap for other integral sizes (less efficient but works)
            std::vector<uint8_t> bytes(sizeof(T));
            std::memcpy(bytes.data(), &value, sizeof(T));
            std::reverse(bytes.begin(), bytes.end());
            T result;
            std::memcpy(&result, bytes.data(), sizeof(T));
            return result;
        }
    }
    return value; // No swap needed if native is big-endian
}


// Function to serialize any trivially copyable type to big-endian bytes
template <typename T>
requires std::is_trivially_copyable_v<T>
void serialize_to_big_endian(const T& value, std::vector<uint8_t>& out) {
    if constexpr (std::is_integral_v<T> && !std::is_same_v<T, bool>) {
        // Handle integral types directly
        T big_endian_val = to_big_endian_integral(value);
        const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&big_endian_val);
        out.insert(out.end(), bytes, bytes + sizeof(T));
    } else if constexpr (std::is_floating_point_v<T>) {
        // Handle floating-point types (float, double, long double)
        // Use std::bit_cast to treat float as an integer of the same size
        if constexpr (sizeof(T) == sizeof(uint32_t)) { // For float
            uint32_t int_repr = std::bit_cast<uint32_t>(value);
            uint32_t big_endian_int = to_big_endian_integral(int_repr);
            const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&big_endian_int);
            out.insert(out.end(), bytes, bytes + sizeof(uint32_t));
        } else if constexpr (sizeof(T) == sizeof(uint64_t)) { // For double
            uint64_t int_repr = std::bit_cast<uint64_t>(value);
            uint64_t big_endian_int = to_big_endian_integral(int_repr);
            const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&big_endian_int);
            out.insert(out.end(), bytes, bytes + sizeof(uint64_t));
        } else {
            // Fallback for other floating point types (e.g., long double) or unsupported sizes
            // Consider specific handling or assertion here.
            std::cout << "Warning: Unsupported floating point size for bit_cast and endian swap." << std::endl;
            // For now, just copy raw bytes (will be platform native endianness)
            const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&value);
            out.insert(out.end(), bytes, bytes + sizeof(T));
        }
    } else if constexpr (std::is_same_v<T, std::complex<float>>) {
        serialize_to_big_endian(value.real(), out);
        serialize_to_big_endian(value.imag(), out);
    } else if constexpr (std::is_same_v<T, std::complex<double>>) {
        serialize_to_big_endian(value.real(), out);
        serialize_to_big_endian(value.imag(), out);
    }
    else {
        // For other trivially copyable types where byte order isn't a concern
        // or you don't need to swap them (e.g., char, uint8_t, or just raw memory dump)
        const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&value);
        out.insert(out.end(), bytes, bytes + sizeof(T));
    }
}

template <typename T>
std::vector<uint8_t> serialize_uniform_vector(const std::vector<T>& vec) {
    std::vector<uint8_t> out;

    write_u8(out, static_cast<uint8_t>(legacy_tag::LEGACY_PMT_UNIFORM_VECTOR));
    write_u8(out, static_cast<uint8_t>(legacy_uniform_type_for<T>()));
    write_u32(out, static_cast<uint32_t>(vec.size()));
    // Padding
    write_u8(out, static_cast<uint8_t>(1));
    write_u8(out, static_cast<uint8_t>(0));

    // out.insert(out.end(),
    //        reinterpret_cast<const uint8_t*>(vec.data()),
    //        reinterpret_cast<const uint8_t*>(vec.data()) + vec.size() * sizeof(T)); 

    for (const auto& val : vec) {
        serialize_to_big_endian(val, out);
    }


    return out;
}

template <typename T>
std::vector<uint8_t> serialize_uniform_vector(const pmtv::Tensor<T>& vec) {
    std::vector<uint8_t> out;

    write_u8(out, static_cast<uint8_t>(legacy_tag::LEGACY_PMT_UNIFORM_VECTOR));
    write_u8(out, static_cast<uint8_t>(legacy_uniform_type_for<T>()));
    write_u32(out, static_cast<uint32_t>(vec.size()));
    // Padding
    write_u8(out, static_cast<uint8_t>(1));
    write_u8(out, static_cast<uint8_t>(0));

    // out.insert(out.end(),
    //        reinterpret_cast<const uint8_t*>(vec.data()),
    //        reinterpret_cast<const uint8_t*>(vec.data()) + vec.size() * sizeof(T)); 

    for (const auto& val : vec) {
        serialize_to_big_endian(val, out);
    }


    return out;
}
// std::vector<uint8_t> serialize_map(const map_t& m) {
//     std::vector<uint8_t> result;

//     // Legacy tag for map might be something like 0x13 — adjust as needed
//     result.push_back(0x13); 

//     // Write number of elements (big-endian uint32)
//     uint32_t size = static_cast<uint32_t>(m.size());
//     result.push_back((size >> 24) & 0xff);
//     result.push_back((size >> 16) & 0xff);
//     result.push_back((size >> 8) & 0xff);
//     result.push_back(size & 0xff);

//     for (const auto& [key, value] : m) {
//         // Serialize key as symbol or string
//         std::vector<uint8_t> key_data = serialize_string_key(key);  // define this
//         result.insert(result.end(), key_data.begin(), key_data.end());

//         // Serialize value
//         std::vector<uint8_t> value_data = serialize_to_legacy(value);
//         result.insert(result.end(), value_data.begin(), value_data.end());
//     }

//     return result;
// }


// --- Serialization: basic types ---
std::vector<uint8_t> serialize_to_legacy(const pmtv::pmt& obj) {

    return std::visit([](const auto& val) -> std::vector<uint8_t> {
        using T = std::decay_t<decltype(val)>;

        std::vector<uint8_t> out;

        if constexpr (std::same_as<T, std::monostate>){
            write_u8(out, static_cast<uint8_t>(legacy_tag::LEGACY_PMT_NULL));
        }
        else if constexpr (std::same_as<T, bool>){
            if (val) {
                write_u8(out, static_cast<uint8_t>(legacy_tag::LEGACY_PMT_TRUE));
            } else {
                write_u8(out, static_cast<uint8_t>(legacy_tag::LEGACY_PMT_FALSE));
            }            
        }
        else if constexpr (std::integral<T>){
            out = serialize_integral(val);
        }
        else if constexpr (std::floating_point<T>) {
            out = serialize_real(val);
        }
        else if constexpr (std::same_as<T, std::string>) {
            out = serialize_string(val);
        }

        else if constexpr (std::ranges::range<T>) {
            if constexpr (UniformVector<T>) {
                out = serialize_uniform_vector(val);
            } else {
                // out = serialize_pmt_vector(val);
            }
        }      
        if constexpr (std::is_same_v<T, map_t>) {
            throw std::runtime_error("No Legacy serialization defined for pmt::dict");
        }

        return out;
    }, obj);

    std::vector<uint8_t> out;

    // //if (obj.is_none()) {
    // if (std::holds_alternative<std::monostate>(obj)) {
    //     write_u8(out, static_cast<uint8_t>(legacy_tag::LEGACY_PMT_NULL));
    // // } else if (obj.is_boolean()) {
    // } else if (std::holds_alternative<bool>(obj)) {
    //     if (pmtv::cast<bool>(obj)) {
    //         write_u8(out, static_cast<uint8_t>(legacy_tag::LEGACY_PMT_TRUE));
    //     } else {
    //         write_u8(out, static_cast<uint8_t>(legacy_tag::LEGACY_PMT_FALSE));
    //     }
    //     // write_u8(out, pmtv::cast<bool>(obj) ? 0 : 1); // legacy pmt serializes True as 0 ?!?!?!
    // } else if (std::holds_alternative<int32_t>(obj)) {
    //     write_u8(out, static_cast<uint8_t>(legacy_tag::LEGACY_PMT_INT32));
    //     write_u32(out, static_cast<uint32_t>(pmtv::cast<int32_t>(obj)));
    // } else if (std::holds_alternative<int64_t>(obj)) {
    //     write_u8(out, static_cast<uint8_t>(legacy_tag::LEGACY_PMT_INT64));
    //     write_u64(out, static_cast<uint64_t>(pmtv::cast<int64_t>(obj)));
    // } else if (std::holds_alternative<double>(obj)) {
    //     write_u8(out, static_cast<uint8_t>(legacy_tag::LEGACY_PMT_DOUBLE));
    //     write_double(out, pmtv::cast<double>(obj));
    // } else if (std::holds_alternative<std::string>(obj)) {
    //     write_u8(out, static_cast<uint8_t>(legacy_tag::LEGACY_PMT_SYMBOL));
    //     const auto& str = pmtv::cast<std::string>(obj);
    //     write_u16(out, static_cast<uint32_t>(str.size()));
    //     out.insert(out.end(), str.begin(), str.end());
    // } else {
    //     throw std::runtime_error("Unsupported pmtv::pmt type for legacy serialization (yet)");
    // }

    return out;
}


template <typename T>
requires std::is_integral_v<T> && (!std::is_same_v<T, bool>)
T from_big_endian_integral_to_native(T value) {
    if constexpr (std::endian::native == std::endian::little) {
        if constexpr (sizeof(T) == 1) {
            return value; // Single byte, no swap needed
        } else if constexpr (sizeof(T) == 2) {
            return static_cast<T>(
                ((static_cast<uint16_t>(value) & 0xFF00) >> 8) |
                ((static_cast<uint16_t>(value) & 0x00FF) << 8)
            );
        } else if constexpr (sizeof(T) == 4) {
            return static_cast<T>(
                ((static_cast<uint32_t>(value) & 0xFF000000) >> 24) |
                ((static_cast<uint32_t>(value) & 0x00FF0000) >> 8) |
                ((static_cast<uint32_t>(value) & 0x0000FF00) << 8) |
                ((static_cast<uint32_t>(value) & 0x000000FF) << 24)
            );
        } else if constexpr (sizeof(T) == 8) {
            uint64_t u64_val = static_cast<uint64_t>(value);
            return static_cast<T>(
                ((u64_val & 0xFF00000000000000ULL) >> 56) |
                ((u64_val & 0x00FF000000000000ULL) >> 40) |
                ((u64_val & 0x0000FF0000000000ULL) >> 24) |
                ((u64_val & 0x000000FF00000000ULL) >> 8)  |
                ((u64_val & 0x00000000FF000000ULL) << 8)  |
                ((u64_val & 0x0000000000FF0000ULL) << 24) |
                ((u64_val & 0x000000000000FF00ULL) << 40) |
                ((u64_val & 0x00000000000000FFULL) << 56)
            );
        } else {
            // Generic byte swap for other integral sizes
            std::vector<uint8_t> bytes(sizeof(T));
            std::memcpy(bytes.data(), &value, sizeof(T));
            std::reverse(bytes.begin(), bytes.end()); // Reverse to convert big-endian to native
            T result;
            std::memcpy(&result, bytes.data(), sizeof(T));
            return result;
        }
    }
    return value; // No swap needed if native is big-endian
}


// Function to deserialize from a big-endian byte stream into a single object of type T
template <typename T>
requires std::is_trivially_copyable_v<T>
T deserialize_from_big_endian(const uint8_t*& ptr) { // ptr is passed by reference to advance it
    T result;

    if constexpr (std::is_integral_v<T> && !std::is_same_v<T, bool>) {
        // Read bytes directly into an integral variable
        T temp_val;
        std::memcpy(&temp_val, ptr, sizeof(T));
        result = from_big_endian_integral_to_native(temp_val);
    } else if constexpr (std::is_floating_point_v<T>) {
        // Use std::bit_cast for floating-point types
        if constexpr (sizeof(T) == sizeof(uint32_t)) { // For float
            uint32_t int_repr;
            std::memcpy(&int_repr, ptr, sizeof(uint32_t));
            uint32_t native_endian_int = from_big_endian_integral_to_native(int_repr);
            result = std::bit_cast<T>(native_endian_int);
        } else if constexpr (sizeof(T) == sizeof(uint64_t)) { // For double
            uint64_t int_repr;
            std::memcpy(&int_repr, ptr, sizeof(uint64_t));
            uint64_t native_endian_int = from_big_endian_integral_to_native(int_repr);
            result = std::bit_cast<T>(native_endian_int);
        } else {
            // Fallback for other floating point types
            std::cout << "Warning: Unsupported floating point size for deserialization." << std::endl;
            // For now, just copy raw bytes (will be platform native endianness if not swapped)
            std::memcpy(&result, ptr, sizeof(T));
        }
    } else if constexpr (std::is_same_v<T, std::complex<float>>) {
        // Deserialize real and imaginary parts separately
        float real_part = deserialize_from_big_endian<float>(ptr);
        float imag_part = deserialize_from_big_endian<float>(ptr);
        result = std::complex<float>(real_part, imag_part);
        // Important: ptr was advanced by inner calls, so we don't advance it again here.
        return result; // Return early because ptr is already advanced
    } else if constexpr (std::is_same_v<T, std::complex<double>>) {
        // Deserialize real and imaginary parts separately
        double real_part = deserialize_from_big_endian<double>(ptr);
        double imag_part = deserialize_from_big_endian<double>(ptr);
        result = std::complex<double>(real_part, imag_part);
        // Important: ptr was advanced by inner calls, so we don't advance it again here.
        return result; // Return early
    }
    // For other trivially copyable types (e.g., structs that are themselves flat byte representations
    // and don't need internal member endian conversion), just copy directly.
    // If a struct has internal members that require endian conversion, you'd need to
    // deserialize each member individually.
    else {
        // Default: just copy the bytes directly
        std::memcpy(&result, ptr, sizeof(T));
    }

    // Advance the pointer for the next read, unless it's handled by recursive calls
    ptr += sizeof(T);
    return result;
}

// Main function to create the vector
template <typename VTYPE>
std::vector<VTYPE> create_vector_from_big_endian(const uint8_t* ptr, size_t num_elements) {
    std::vector<VTYPE> vec;
    vec.reserve(num_elements); // Pre-allocate for efficiency

    for (size_t i = 0; i < num_elements; ++i) {
        vec.push_back(deserialize_from_big_endian<VTYPE>(ptr));
    }
    return vec;
}



// --- Deserialization: basic types ---
pmtv::pmt deserialize_from_legacy(const uint8_t* data, size_t size) {
    const uint8_t* ptr = data;

    if (size == 0)
        throw std::runtime_error("Empty legacy PMT buffer");

    auto tag = static_cast<legacy_tag>(*ptr++);
    pmtv::pmt ret;
    switch (tag) {
        case legacy_tag::LEGACY_PMT_NULL:
            return ret;
        case legacy_tag::LEGACY_PMT_TRUE:
            ret = true;
            return ret;
        case legacy_tag::LEGACY_PMT_FALSE:
            ret = false;
            return ret;
        case legacy_tag::LEGACY_PMT_INT32:
            ret = static_cast<int32_t>(read_u32(ptr));
            return ret;
        case legacy_tag::LEGACY_PMT_INT64:
            ret = static_cast<int64_t>(read_u64(ptr));
            return ret;
        case legacy_tag::LEGACY_PMT_DOUBLE:
            ret = read_double(ptr);
            return ret;
        case legacy_tag::LEGACY_PMT_SYMBOL: {
            uint16_t len = (ptr[0] << 8) | (ptr[1] << 0);
            ptr += 2;
            std::string sym(reinterpret_cast<const char*>(ptr), len);
            ptr += len;
            ret = sym;
            return ret;
        }
        case legacy_tag::LEGACY_PMT_UNIFORM_VECTOR: {
            legacy_uniform_type dtype = static_cast<legacy_uniform_type>(ptr[0]);
            ptr += 1;
            uint16_t len = read_u32(ptr); // ptr is incremented inside read_u32
            uint8_t npad = ptr[0]; ptr += 1;
            ptr += npad;

            switch (dtype) {
                case legacy_uniform_type::U8: {
                    using VTYPE = uint8_t;
                    std::vector<VTYPE> vec = create_vector_from_big_endian<VTYPE>(ptr, len);
                    ret = pmtv::Tensor<VTYPE>(vec);
                    break;
                }
                case legacy_uniform_type::S8: {
                    using VTYPE = int8_t;
                    std::vector<VTYPE> vec = create_vector_from_big_endian<VTYPE>(ptr, len);
                    ret = pmtv::Tensor<VTYPE>(vec); break;
                }
                case legacy_uniform_type::U16: {
                    using VTYPE = uint16_t;
                    std::vector<VTYPE> vec = create_vector_from_big_endian<VTYPE>(ptr, len);
                    ret = pmtv::Tensor<VTYPE>(vec); break;
                }
                case legacy_uniform_type::S16: {
                    using VTYPE = int16_t;
                    std::vector<VTYPE> vec = create_vector_from_big_endian<VTYPE>(ptr, len);
                    ret = pmtv::Tensor<VTYPE>(vec); break;
                }
                case legacy_uniform_type::U32: {
                    using VTYPE = uint32_t;
                    std::vector<VTYPE> vec = create_vector_from_big_endian<VTYPE>(ptr, len);
                    ret = pmtv::Tensor<VTYPE>(vec); break;
                }
                case legacy_uniform_type::S32: {
                    using VTYPE = int32_t;
                    std::vector<VTYPE> vec = create_vector_from_big_endian<VTYPE>(ptr, len);
                    ret = pmtv::Tensor<VTYPE>(vec); break;
                }
                case legacy_uniform_type::U64: {
                    using VTYPE = uint64_t;
                    std::vector<VTYPE> vec = create_vector_from_big_endian<VTYPE>(ptr, len);
                    ret = pmtv::Tensor<VTYPE>(vec); break;
                }
                case legacy_uniform_type::S64: {
                    using VTYPE = int64_t;
                    std::vector<VTYPE> vec = create_vector_from_big_endian<VTYPE>(ptr, len);
                    ret = pmtv::Tensor<VTYPE>(vec); break;
                }                                                                                                                           
                case legacy_uniform_type::F32: {
                    using VTYPE = float;
                    std::vector<VTYPE> vec = create_vector_from_big_endian<VTYPE>(ptr, len);
                    ret = pmtv::Tensor<VTYPE>(vec); break;
                }    
                case legacy_uniform_type::F64: {
                    using VTYPE = double;
                    std::vector<VTYPE> vec = create_vector_from_big_endian<VTYPE>(ptr, len);
                    ret = pmtv::Tensor<VTYPE>(vec); break;
                }   
                case legacy_uniform_type::C32: {
                    using VTYPE = std::complex<float>;
                    std::vector<VTYPE> vec = create_vector_from_big_endian<VTYPE>(ptr, len);
                    ret = pmtv::Tensor<VTYPE>(vec); break;
                }    
                case legacy_uniform_type::C64: {
                    using VTYPE = std::complex<double>;
                    std::vector<VTYPE> vec = create_vector_from_big_endian<VTYPE>(ptr, len);
                    ret = pmtv::Tensor<VTYPE>(vec); break;
                }             
                default: {
                    throw std::runtime_error("Unsupported or unknown legacy PMT uniform vector tag");
                }                      
            }
            return ret;


        }
        case legacy_tag::LEGACY_PMT_DICT: {
            throw std::runtime_error("No Deserialization defined for Legacy pmt dict");
        }
        default:
            throw std::runtime_error("Unsupported or unknown legacy PMT tag");
    }
}

} // namespace legacy_pmt
