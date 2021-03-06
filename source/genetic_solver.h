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
#ifndef GENETIC_SOLVER_H
#define GENETIC_SOLVER_H

#include <iostream>

#include "chromosome.h"
#include "population.h"

namespace GeneticAlgorithms {

  /**
   * This function implements a generic genetic algorithm
   *
   * This algorithm is build on top of several genetic operators:
   *
   * - InitializerFunctor: a functor which returns a Chromosome each
   *      time it is called.
   *
   * - SelectionFunctor: a functor which receives a vector of
   *      hypothesis and produces as output a vector of
   *      Chromosome::Couple selected for cross over.
   *
   * - CrossOverFunctor: a functor which receives two Chromosome and
   *      returns their child, mixing gens on both inputs.
   *
   * - MutationFunctor: a functor which receives a Chromosome and
   *      returns another one with some gens mutated (or not).
   *
   * - RankFunctor: a functor which receives a Chromosome and returns
   *      its rank (template typename T)
   *
   * ATTENTION this function maximizes by default, change RankFunctor
   * sign for minimization.
   *
   * @note This function implements basic elitism algorithm, the best
   * candidate survives to next generation.
   *
   * @code
   *  struct MyRank {
   *    float operator()(const Chromosome &x) const {
   *      Decoder decoder(x);
   *      float rank = decoder.decodeFloat(N, -5.0f, 5.0f);
   *      return rank;
   *    }
   *  };
   *  std::mt19937_64 rng(12564);
   *  Chromosome best = solve(1000u, // iterations
   *                          100u,  // population
   *                          RandomInitializer(N, rng(), 0.5f),
   *                          FloatRouletteWheelSelection(rng()),
   *                          RandomSplitCrossOver(N, rng()),
   *                          RandomMutate(rng(), 0.5f),
   *                          MyRank());
   * @endcode
   */
  template<typename InitializerFunctor,
           typename SelectionFunctor,
           typename CrossOverFunctor,
           typename MutationFunctor,
           typename RankFunctor,
           typename T=float>
  Chromosome solve(const size_t num_iterations,
                   const size_t population_size,
                   const InitializerFunctor &init_func,
                   const SelectionFunctor &select_func,
                   const CrossOverFunctor &cross_over_func,
                   const MutationFunctor &mutate_func,
                   const RankFunctor &rank_func,
                   int verbosity=0) {
    Population<RankFunctor, T> current(rank_func);
    Population<RankFunctor, T> next(rank_func);

    current.init(init_func, population_size);

    typename Population<RankFunctor, T>::Hypothesis best = current.top();

    for (size_t i=0; i<num_iterations; ++i) {
      for (auto couple : current.select(select_func, population_size - 1uL)) {
        next.push(mutate_func(cross_over_func(couple.first, couple.second)));
      }
      std::swap(current, next);
      next.reset();
      if (best.second < current.top().second) {
        best = current.top();
      }
      // elitism: the best one passes directly
      current.push(best.first);
    }

    return best.first;
  }

} // namespace GeneticAlgorithms

#endif // GENETIC_SOLVER_H
