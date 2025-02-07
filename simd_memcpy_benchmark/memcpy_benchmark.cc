#include <immintrin.h>  // For AVX intrinsics (adjust for your architecture)

#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstdlib>  // For posix_memalign
#include <cstring>
#include <iostream>
#include <vector>

// Function to perform aligned memcpy
void aligned_memcpy(void* dest, const void* src, size_t size) {
  memcpy(dest, src, size);
}

// Function to perform unaligned memcpy
// In practice, memcpy implementations often have internal checks and
// optimized paths for unaligned cases, but we'll force unaligned pointers here
// for demonstration.  The behavior will be similar to a naive copy loop.
void unaligned_memcpy(void* dest, const void* src, size_t size) {
  char* d = reinterpret_cast<char*>(dest);
  const char* s = reinterpret_cast<const char*>(src);
  for (size_t i = 0; i < size; ++i) {
    d[i] = s[i];
  }
}

// Function to perform aligned SIMD memcpy (using AVX2 as an example)
void aligned_simd_memcpy(void* dest, const void* src, size_t size) {
  char* d = reinterpret_cast<char*>(dest);
  const char* s = reinterpret_cast<const char*>(src);

  // Handle the bulk of the data with AVX2
  size_t i = 0;
  for (; i + 32 <= size; i += 32) {
    __m256i data = _mm256_load_si256(reinterpret_cast<const __m256i*>(s + i));
    _mm256_store_si256(reinterpret_cast<__m256i*>(d + i), data);
  }

  // Handle any remaining data with standard memcpy (or a scalar loop)
  memcpy(d + i, s + i, size - i);
}

// Function to perform unaligned SIMD memcpy (using AVX2 as an example)
void unaligned_simd_memcpy(void* dest, const void* src, size_t size) {
  char* d = reinterpret_cast<char*>(dest);
  const char* s = reinterpret_cast<const char*>(src);

  size_t i = 0;
  // Use unaligned loads and stores.  Note: Requires AVX (not just AVX2).
  for (; i + 32 <= size; i += 32) {
    __m256i data = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(s + i));
    _mm256_storeu_si256(reinterpret_cast<__m256i*>(d + i), data);
  }

  // Handle any remaining data.
  for (; i < size; ++i) {
    d[i] = s[i];
  }
}

int main() {
  size_t size = 1024 * 1024 * 100 ;  // 100 MB
  size_t alignment = 64;            // Align to 64-byte cache line boundary

  void *src_aligned, *dest_aligned;
  void *src_unaligned, *dest_unaligned;

  // Allocate aligned memory
  if (posix_memalign(&src_aligned, alignment, size) != 0 ||
      posix_memalign(&dest_aligned, alignment, size) != 0) {
    std::cerr << "Failed to allocate aligned memory" << std::endl;
    return 1;
  }

  // Create unaligned pointers by offsetting from the aligned pointers
  char* src_aligned_char = reinterpret_cast<char*>(src_aligned);
  char* dest_aligned_char = reinterpret_cast<char*>(dest_aligned);

  src_unaligned = src_aligned_char + 1;
  dest_unaligned = dest_aligned_char + 3;  // Different offset for dest

  // Initialize source buffer with some data
  for (size_t i = 0; i < size; ++i) {
    reinterpret_cast<char*>(src_aligned)[i] = (char)(i % 256);
  }
  // Ensure that the unaligned buffer has different content than the aligned
  // buffer.
  reinterpret_cast<char*>(src_unaligned)[0] = 'X';

  // --- Aligned memcpy ---
  auto start = std::chrono::high_resolution_clock::now();
  aligned_memcpy(dest_aligned, src_aligned, size);
  auto end = std::chrono::high_resolution_clock::now();
  auto duration_aligned_memcpy =
      std::chrono::duration_cast<std::chrono::microseconds>(end - start)
          .count();

  int memcmp_aligned = 0, memcmp_unaligned = 0;

  // Check if copies were successful
  memcmp_aligned = memcmp(src_aligned, dest_aligned, size);

  std::cout << "Aligned memcpy time: " << duration_aligned_memcpy << " us"
            << std::endl;
  std::cout << "Aligned memcpy result correct: "
            << (memcmp_aligned == 0 ? "true" : "false") << std::endl;

  // --- Unaligned memcpy ---
  start = std::chrono::high_resolution_clock::now();
  unaligned_memcpy(dest_unaligned, src_unaligned, size);
  end = std::chrono::high_resolution_clock::now();
  auto duration_unaligned_memcpy =
      std::chrono::duration_cast<std::chrono::microseconds>(end - start)
          .count();

  // Check if copies were successful
  memcmp_unaligned = memcmp(src_unaligned, dest_unaligned, size - 3);

  std::cout << "Unaligned memcpy time: " << duration_unaligned_memcpy << " us"
            << std::endl;
  std::cout << "Unaligned memcpy result correct: "
            << (memcmp_unaligned == 0 ? "true" : "false") << std::endl;

  // --- Aligned SIMD memcpy ---
  start = std::chrono::high_resolution_clock::now();
  aligned_simd_memcpy(dest_aligned, src_aligned, size);
  end = std::chrono::high_resolution_clock::now();
  auto duration_aligned_simd =
      std::chrono::duration_cast<std::chrono::microseconds>(end - start)
          .count();

  // Check if copies were successful
  memcmp_aligned = memcmp(src_aligned, dest_aligned, size);

  std::cout << "Aligned SIMD memcpy time: " << duration_aligned_simd << " us"
            << std::endl;
  std::cout << "Aligned memcpy result correct: "
            << (memcmp_aligned == 0 ? "true" : "false") << std::endl;

  // --- Unaligned SIMD memcpy ---
  start = std::chrono::high_resolution_clock::now();
  unaligned_simd_memcpy(dest_unaligned, src_unaligned, size);
  end = std::chrono::high_resolution_clock::now();
  auto duration_unaligned_simd =
      std::chrono::duration_cast<std::chrono::microseconds>(end - start)
          .count();

  // Check if copies were successful
  memcmp_unaligned = memcmp(src_unaligned, dest_unaligned, size - 3);

  std::cout << "Unaligned SIMD memcpy time: " << duration_unaligned_simd
            << " us" << std::endl;
  std::cout << "Unaligned memcpy result correct: "
            << (memcmp_unaligned == 0 ? "true" : "false") << std::endl;

  free(src_aligned);
  free(dest_aligned);

  return 0;
}