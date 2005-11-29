#include "ac_cache_if.H"

void bind(ac_cache_if& previous, ac_cache_if& next) {
  previous.bindToNext(next);
  next.bindToPrevious(previous);
}
