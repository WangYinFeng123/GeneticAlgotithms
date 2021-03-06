/*
 * This file is part of GeneticAlgorithms toolkit
 *
 * Copyright 2017, Francisco Zamora-Martinez
 *
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef CROSSOVERS_H
#define CROSSOVERS_H

#include <algorithm>
#include <boost/dynamic_bitset.hpp>
#include <random>

#include "chromosome.h"

namespace GeneticAlgorithms {

  /**
   * A cross over class based on random position-based split
   *
   * The functor implemented here splits the chromosomes by sampling a
   * random integer, and producing a child which mixes together one
   * piece of one parent and the other from the other parent.
   *
   * ATTENTION: no thread safe object, it should be created for each
   * thread in your program.
   */
  class RandomSplitCrossOver {
  public:
    RandomSplitCrossOver(size_t N, unsigned seed) :
      _rng(seed),
      _int_dist(0uL, N-1),
      _binary_dist(0uL, 1uL) {
    }

    Chromosome operator()(const Chromosome &a, const Chromosome &b) const {
      bitset dest(a.size());
      // sample a random integer
      size_t pos = static_cast<size_t>(_int_dist(_rng));
      if (_binary_dist(_rng) == 0uL) {
        for (size_t i=0; i<pos; ++i) {
          dest[i] = a[i];
        }
        for (size_t i=pos; i<b.size(); ++i) {
          dest[i] = b[i];
        }
      }
      else {
        for (size_t i=0; i<pos; ++i) {
          dest[i] = b[i];
        }
        for (size_t i=pos; i<a.size(); ++i) {
          dest[i] = a[i];
        }
      }
      return Chromosome(std::move(dest));
    }
  private:
    mutable std::mt19937_64 _rng;
    mutable std::uniform_int_distribution<size_t> _int_dist;
    mutable std::uniform_int_distribution<size_t> _binary_dist;
  }; // class RandomSplitCrossOver


  /**
   * A cross over class based on random mixing of gens
   *
   * The functor implemented here mixes each gene based on a
   * random decision, so each gene has 0.5 probability to come
   * from any of both parents.
   *
   * ATTENTION: no thread safe object, it should be created for each
   * thread in your program.
   */
  class RandomMixCrossOver {
  public:
    RandomMixCrossOver(unsigned seed) :
      _rng(seed),
      _int_dist(0u, 1u) {
    }

    Chromosome operator()(const Chromosome &a, const Chromosome &b) const {
      bitset dest(a.size());
      for (size_t i=0; i<a.size(); ++i) {
        // flip a coin to decide which parent gene copy is at i position
        if (_int_dist(_rng) == 0u) {
          dest[i] = a[i];
        }
        else {
          dest[i] = b[i];
        }
      }
      return Chromosome(std::move(dest));
    }
  private:
    mutable std::mt19937_64 _rng;
    mutable std::uniform_int_distribution<size_t> _int_dist;
  }; // class RandomMixCrossOver


  /**
   * This class introduces cross-over probability over cross-over functors
   *
   * Instead of instantiated directly this class, use the helper function
   * make_cross_over_on_prob
   */
  template <typename CrossOverFunctor>
  class CrossOverOnProbWrapper {
  public:
    CrossOverOnProbWrapper(unsigned seed, float prob,
                           const CrossOverFunctor &crossover) :
      _rng(seed),
      _real_dist(0.0f, 1.0f),
      _binary_dist(0uL, 1uL),
      _prob(prob),
      _crossover(crossover) {
    }

    /// Cross-overs with _prob probability, else returns one random parent
    Chromosome operator()(const Chromosome &a, const Chromosome &b) const {
      if (_real_dist(_rng) < _prob) {
        return _crossover(a, b);
      }
      else {
        if (_binary_dist(_rng) == 0uL) return a;
        else return b;
      }
    }

  private:
    mutable std::mt19937_64 _rng;
    mutable std::uniform_real_distribution<float> _real_dist;
    mutable std::uniform_int_distribution<size_t> _binary_dist;
    float _prob;
    CrossOverFunctor _crossover;
  };

  /// Helper for construction of CrossOverOnProbWrapper instances
  template <typename CrossOverFunctor>
  CrossOverOnProbWrapper<CrossOverFunctor>
  make_cross_over_on_prob(unsigned seed,
                          float prob,
                          const CrossOverFunctor &crossover) {
    return CrossOverOnProbWrapper<CrossOverFunctor>(seed,
                                                    prob,
                                                    crossover);
  }

} // namespace GeneticAlgorithms

#endif // CROSSOVERS_H
