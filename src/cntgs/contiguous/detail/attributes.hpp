#ifndef CNTGS_DETAIL_ATTRIBUTES_HPP
#define CNTGS_DETAIL_ATTRIBUTES_HPP

#ifdef NDEBUG
#ifdef _MSC_VER
#define CNTGS_FORCE_INLINE __forceinline
#else
#define CNTGS_FORCE_INLINE __attribute__((always_inline)) inline
#endif
#else
#define CNTGS_FORCE_INLINE inline
#endif

#ifdef _MSC_VER
#define CNTGS_RESTRICT_RETURN __declspec(restrict)
#else
#define CNTGS_RESTRICT_RETURN __attribute__((malloc)) __attribute__((returns_nonnull))
#endif

#define CNTGS_RESTRICT __restrict

#endif  // CNTGS_DETAIL_ATTRIBUTES_HPP
