template <endianness big_endian>
std::pair<const char* const, char* const> sse_convert_latin1_to_utf8(
  const char* latin_input,
  const size_t latin_input_length,
  char* utf8_output) {
  // 0b1000_0000
  const __m128i v_80 = _mm_set1_epi8((uint8_t)0x80);
  const __m128i v_0000 = _mm_setzero_si128();
  // 0b1111_1111_1000_0000
  const __m128i v_ff80 = _mm_set1_epi16((uint16_t)0xff80);

  const __m128i latin_1_half_into_u16_byte_mask = big_endian
    ? _mm_setr_epi8(
      '\x80', 0,
      '\x80', 1,
      '\x80', 2,
      '\x80', 3,
      '\x80', 4,
      '\x80', 5,
      '\x80', 6,
      '\x80', 7
    )
    : _mm_setr_epi8(
      0, '\x80', 
      1, '\x80', 
      2, '\x80', 
      3, '\x80', 
      4, '\x80', 
      5, '\x80',
      6, '\x80',
      7, '\x80'
    );

  const __m128i latin_2_half_into_u16_byte_mask = big_endian
    ? _mm_setr_epi8(
      '\x80', 8,
      '\x80', 9,
      '\x80', 10,
      '\x80', 11,
      '\x80', 12,
      '\x80', 13,
      '\x80', 14,
      '\x80', 15
    )
    : _mm_setr_epi8(
      8, '\x80',
      9, '\x80',
      10, '\x80',
      11, '\x80',
      12, '\x80',
      13, '\x80',
      14, '\x80',
      15, '\x80'
    );


  const char* end = latin_input + latin_input_length;

  while (latin_input + 16 <= end) {
    // Load 16 Latin1 characters (16 bytes) into a 128-bit register
    __m128i v_latin = _mm_loadu_si128((__m128i*)latin_input);

    if (_mm_testz_si128(v_latin, v_80)) {// ASCII fast path!!!!
      _mm_storeu_si128((__m128i*)utf8_output, v_latin);
      latin_input += 16;
      utf8_output += 16;
      continue;
    }

    // assuming a/b are bytes and A/B are uint16 of the same value
    // aaaa_aaaa_bbbb_bbbb -> AAAA_AAAA
    __m128i v_u16_latin_1_half = _mm_shuffle_epi8(v_latin, latin_1_half_into_u16_byte_mask);
    // aaaa_aaaa_bbbb_bbbb -> BBBB_BBBB
    __m128i v_u16_latin_2_half = _mm_shuffle_epi8(v_latin, latin_2_half_into_u16_byte_mask);


    latin_input += internal::westmere::write_v_u16_11bits_to_utf8(v_u16_latin_1_half, utf8_output, v_0000, v_ff80);
    latin_input += internal::westmere::write_v_u16_11bits_to_utf8(v_u16_latin_2_half, utf8_output, v_0000, v_ff80);
  }

  return std::make_pair(latin_input, utf8_output);
};