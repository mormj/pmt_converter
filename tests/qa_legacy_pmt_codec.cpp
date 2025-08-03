#include <gtest/gtest.h>
#include <pmt_legacy_codec.h>
#include <vector>

namespace {

    const std::vector<uint8_t> legacy_nil_data = {0x6};
    const std::vector<uint8_t> legacy_bool_data = {0x0}; // true
    const std::vector<uint8_t> legacy_int32_data = {0x03,0x00,0x00,0x00,0x2a}; //42
    // >>> pmt.serialize_str(pmt.from_long(249387429783478)).hex()    
    const std::vector<uint8_t> legacy_int64_data = {0x0d,0x00,0x00,0xe2,0xd1,0x09,0x29,0xe7,0xb6}; //42
    // >>> pmt.serialize_str(pmt.from_double(3.14159)).hex()
    const std::vector<uint8_t> legacy_real_data = {0x04,0x40,0x09,0x21,0xf9,0xf0,0x1b,0x86,0x6e}; // 3.14159

    const std::vector<uint8_t> legacy_symbol_data = {0x02,0x00,0x07,0x65,0x78,0x61,0x6d,0x70,0x6c,0x65}; // "example"
    const std::vector<uint8_t> legacy_complex_data = {0x05,0x40,0x5e,0xdd,0x2f,0x1a,0x9f,0xbe,0x77,0xc0,0x88,0xaa,0x91,0x68,0x72,0xb0,0x21}; //123.456 -1j*789.321
    // >>> pmt.serialize_str(pmt.make_tuple(pmt.from_long(123), pmt.from_double(456.789))).hex()
    const std::vector<uint8_t> legacy_tuple_data = {0x0c,0x00,0x00,0x00,0x02,0x03,0x00,0x00,0x00,0x7b,0x04,0x40,0x7c,0x8c,0x9f,0xbe,0x76,0xc8,0xb4};
    // >>> pmt.serialize_str(pmt.cons(pmt.from_long(123), pmt.from_double(456.789))).hex()
    const std::vector<uint8_t> legacy_pair_data = {0x07,0x03,0x00,0x00,0x00,0x7b,0x04,0x40,0x7c,0x8c,0x9f,0xbe,0x76,0xc8,0xb4};
    // >>> pmt.serialize_str(pmt.make_u8vector(4,222)).hex()   
    const std::vector<uint8_t> legacy_u8vector_data = {0x0a,0x00,0x00,0x00,0x00,0x04,0x01,0x00,0xde,0xde,0xde,0xde};
    // >>> pmt.serialize_str(pmt.make_f32vector(4,-987.654321)).hex()   
    const std::vector<uint8_t> legacy_f32vector_data = {0x0a,0x08,0x00,0x00,0x00,0x04,0x01,0x00,0xc4,0x76,0xe9,0xe0,0xc4,0x76,0xe9,0xe0,0xc4,0x76,0xe9,0xe0,0xc4,0x76,0xe9,0xe0};
    // >>> pmt.serialize_str(pmt.make_c32vector(4,-987.654321+1j*123.456789)).hex()
    const std::vector<uint8_t> legacy_c32vector_data = {0x0a,0x0a,0x00,0x00,0x00,0x04,0x01,0x00,0xc4,0x76,0xe9,0xe0,0x42,0xf6,0xe9,0xe0,0xc4,0x76,0xe9,0xe0,0x42,0xf6,0xe9,0xe0,0xc4,0x76,0xe9,0xe0,0x42,0xf6,0xe9,0xe0,0xc4,0x76,0xe9,0xe0,0x42,0xf6,0xe9,0xe0};

    // >>> d = pmt.make_dict()
    // >>> d = pmt.dict_add(d, pmt.string_to_symbol("spam"), pmt.from_long(42))
    // >>> d = pmt.dict_add(d, pmt.string_to_symbol("eggs"), pmt.from_long(43))
    // >>> pmt.serialize_str(d).hex()
    const std::vector<uint8_t> legacy_dict_data = {0x09,0x07,0x02,0x00,0x04,0x65,0x67,0x67,0x73,0x03,0x00,0x00,0x00,0x2b,0x09,0x07,0x02,0x00,0x04,0x73,0x70,0x61,0x6d,0x03,0x00,0x00,0x00,0x2a,0x06};
    

    // --- Test: Serialize to legacy format ---
    TEST(PmtLegacyCodecTest, SerializeNil) {
        pmtv::pmt obj; // defaults to pmt_null
        std::vector<uint8_t> serialized = legacy_pmt::serialize_to_legacy(obj);
        EXPECT_EQ(serialized, legacy_nil_data);
    }

    TEST(PmtLegacyCodecTest, SerializeBool) {
        pmtv::pmt obj = true;
        std::vector<uint8_t> serialized = legacy_pmt::serialize_to_legacy(obj);
        EXPECT_EQ(serialized, legacy_bool_data);
    }

    TEST(PmtLegacyCodecTest, SerializeInt32) {
        pmtv::pmt obj = 42;
        std::vector<uint8_t> serialized = legacy_pmt::serialize_to_legacy(obj);
        EXPECT_EQ(serialized, legacy_int32_data);
    }

    TEST(PmtLegacyCodecTest, SerializeInt64) {
        pmtv::pmt obj = 249387429783478;
        std::vector<uint8_t> serialized = legacy_pmt::serialize_to_legacy(obj);
        EXPECT_EQ(serialized, legacy_int64_data);
    }

    TEST(PmtLegacyCodecTest, SerializeReal) {
        pmtv::pmt obj = 3.14159d;
        std::vector<uint8_t> serialized = legacy_pmt::serialize_to_legacy(obj);
        EXPECT_EQ(serialized, legacy_real_data);
    }

    TEST(PmtLegacyCodecTest, SerializeSymbol) {
        pmtv::pmt obj = "example";
        std::vector<uint8_t> serialized = legacy_pmt::serialize_to_legacy(obj);
        EXPECT_EQ(serialized, legacy_symbol_data);
    }

    TEST(PmtLegacyCodecTest, SerializeUniformVector) {
        pmtv::pmt obj = std::vector<uint8_t>(4,222);
        std::vector<uint8_t> serialized = legacy_pmt::serialize_to_legacy(obj);
        EXPECT_EQ(serialized, legacy_u8vector_data);

        obj = std::vector<float>(4,-987.654321);
        std::vector<uint8_t> serialized_f32 = legacy_pmt::serialize_to_legacy(obj);
        EXPECT_EQ(serialized_f32, legacy_f32vector_data);

        obj = std::vector<std::complex<float>>(4,{-987.654321,123.456789});
        std::vector<uint8_t> serialized_c32 = legacy_pmt::serialize_to_legacy(obj);
        EXPECT_EQ(serialized_c32, legacy_c32vector_data);        

    }    


    TEST(PmtLegacyCodecTest, SerializeDict) {
        
        pmtv::map_t input_map({
                                  {"spam", static_cast<int>(42)},
                                  {"eggs", static_cast<int>(43)},
                          });
        pmtv::pmt obj = input_map;
        std::vector<uint8_t> serialized = legacy_pmt::serialize_to_legacy(obj);
        EXPECT_EQ(serialized, legacy_dict_data);
    }

    // --- Test: Deserialize from legacy format ---
    TEST(PmtLegacyCodecTest, DeserializeNil) {
        pmtv::pmt obj = legacy_pmt::deserialize_from_legacy(legacy_nil_data.data(), legacy_nil_data.size());
        EXPECT_TRUE(obj == pmtv::pmt_null());
    }

    TEST(PmtLegacyCodecTest, DeserializeBool) {
        pmtv::pmt obj = legacy_pmt::deserialize_from_legacy(legacy_bool_data.data(), legacy_bool_data.size());
        // EXPECT_TRUE(obj.is_boolean());
        EXPECT_TRUE(pmtv::cast<bool>(obj));
    }

    TEST(PmtLegacyCodecTest, DeserializeInt32) {
        pmtv::pmt obj = legacy_pmt::deserialize_from_legacy(legacy_int32_data.data(), legacy_int32_data.size());
        EXPECT_EQ(pmtv::cast<int32_t>(obj), 42);
    }

    TEST(PmtLegacyCodecTest, DeserializeInt64) {
        pmtv::pmt obj = legacy_pmt::deserialize_from_legacy(legacy_int64_data.data(), legacy_int64_data.size());
        EXPECT_EQ(pmtv::cast<int64_t>(obj), 249387429783478);
    }

    TEST(PmtLegacyCodecTest, DeserializeReal) {
        pmtv::pmt obj = legacy_pmt::deserialize_from_legacy(legacy_real_data.data(), legacy_real_data.size());
        EXPECT_NEAR(pmtv::cast<float>(obj), 3.14159, 1e-5);
    }

    TEST(PmtLegacyCodecTest, DeserializeSymbol) {
        pmtv::pmt obj = legacy_pmt::deserialize_from_legacy(legacy_symbol_data.data(), legacy_symbol_data.size());
        EXPECT_EQ(pmtv::cast<std::string>(obj), "example");
    }

    TEST(PmtLegacyCodecTest, DeserializeUniformVector) {
        pmtv::pmt obj = legacy_pmt::deserialize_from_legacy(legacy_u8vector_data.data(), legacy_u8vector_data.size());

        std::vector<uint8_t> expected_u8_vector(4,222);
        EXPECT_EQ(pmtv::cast<std::vector<uint8_t>>(obj), expected_u8_vector);

        obj = legacy_pmt::deserialize_from_legacy(legacy_f32vector_data.data(), legacy_f32vector_data.size());
        std::vector<float> expected_f32_vector(4, -987.654321);
        EXPECT_EQ(pmtv::cast<std::vector<float>>(obj), expected_f32_vector);
        
        obj = legacy_pmt::deserialize_from_legacy(legacy_c32vector_data.data(), legacy_c32vector_data.size());
        std::vector<std::complex<float>> expected_c32_vector(4,{-987.654321,123.456789});
        EXPECT_EQ(pmtv::cast<std::vector<std::complex<float>>>(obj), expected_c32_vector);        
    }

}

