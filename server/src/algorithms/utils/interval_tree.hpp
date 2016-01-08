//
//  interval_tree.hpp
//  DeepBlue Epigenomic Data Server

//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.

//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
//  Created by Fabian Reinartz on 29.08.13.
//  Copyright (c) 2016 Max Planck Institute for Informatics. All rights reserved.

//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.

//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
//  based on: https://github.com/arq5x/intervaltree
//

#ifndef EPIDB_ALGORITHMS_INTERVAL_TREE_H
#define EPIDB_ALGORITHMS_INTERVAL_TREE_H

#include <vector>
#include <algorithm>
#include <iostream>

using namespace std;


template <class T, typename K = int>
class Interval {
public:
  K start;
  K stop;
  T value;
  Interval(K s, K e, T &&v)
    : start(s)
    , stop(e)
    , value(std::move(v))
  { }
};

template <class T, typename K>
int intervalStart(const Interval<T, K> &i)
{
  return i.start;
}

template <class T, typename K>
int intervalStop(const Interval<T, K> &i)
{
  return i.stop;
}

template <class T, typename K>
ostream &operator<<(ostream &out, Interval<T, K> &i)
{
  out << "Interval(" << i.start << ", " << i.stop << "): " << i.value;
  return out;
}

template <class T, typename K = int>
class IntervalStartSorter {
public:
  bool operator() (const Interval<T, K> &a, const Interval<T, K> &b)
  {
    return a.start < b.start;
  }
};

template <class T, typename K = int>
class IntervalTree {

public:
  typedef Interval<T, K> interval;
  typedef vector<interval> intervalVector;
  typedef vector<T> overlapVector;
  typedef IntervalTree<T, K> intervalTree;

  intervalVector intervals;
  intervalTree *left;
  intervalTree *right;
  int center;

  IntervalTree<T, K>(void)
    : left(nullptr)
    , right(nullptr)
    , center(0)
  { }

  IntervalTree<T, K>(const intervalTree &other)
    : intervals(other.intervals)
    , left(nullptr)
    , right(nullptr)
    , center(other.center)
  {
    if (other.left) {
      left = new intervalTree();
      *left = *other.left;
    } else {
      left = NULL;
    }
    if (other.right) {
      right = new intervalTree();
      *right = *other.right;
    } else {
      right = NULL;
    }
  }

  IntervalTree<T, K> &operator=(const intervalTree &other)
  {
    if (*this == &other) {
      return *this;
    }
    center = other.center;
    intervals = other.intervals;
    if (other.left) {
      left = new intervalTree();
      *left = *other.left;
    } else {
      left = NULL;
    }
    if (other.right) {
      right = new intervalTree();
      *right = *other.right;
    } else {
      right = NULL;
    }
    return *this;
  }

  IntervalTree<T, K>(
    intervalVector &ivals,
    unsigned int depth = 16,
    unsigned int minbucket = 64,
    int leftextent = 0,
    int rightextent = 0,
    unsigned int maxbucket = 512
  )
    : left(NULL)
    , right(NULL)
  {

    --depth;
    if (depth == 0 || (ivals.size() < minbucket && ivals.size() < maxbucket)) {
      intervals = std::move(ivals);
    } else {
      if (leftextent == 0 && rightextent == 0) {
        // sort intervals by start
        IntervalStartSorter<T, K> intervalStartSorter;
        sort(ivals.begin(), ivals.end(), intervalStartSorter);
      }

      int leftp = 0;
      int rightp = 0;
      int centerp = 0;

      if (leftextent || rightextent) {
        leftp = leftextent;
        rightp = rightextent;
      } else {
        leftp = ivals.front().start;
        vector<K> stops;
        stops.resize(ivals.size());
        transform(ivals.begin(), ivals.end(), stops.begin(), intervalStop<T, K>);
        rightp = *max_element(stops.begin(), stops.end());
      }

      //centerp = ( leftp + rightp ) / 2;
      centerp = ivals.at(ivals.size() / 2).start;
      center = centerp;

      intervalVector lefts;
      intervalVector rights;

      for (typename intervalVector::iterator i = ivals.begin(); i != ivals.end(); ++i) {
        interval &interval = *i;
        if (interval.stop < center) {
          lefts.push_back(std::move(interval));
        } else if (interval.start > center) {
          rights.push_back(std::move(interval));
        } else {
          intervals.push_back(std::move(interval));
        }
      }

      if (!lefts.empty()) {
        left = new intervalTree(lefts, depth, minbucket, leftp, centerp);
      }
      if (!rights.empty()) {
        right = new intervalTree(rights, depth, minbucket, centerp, rightp);
      }
    }
  }

  void findOverlapping(K start, K stop, overlapVector &overlapping)
  {
    if (!intervals.empty() && ! (stop < intervals.front().start)) {
      for (typename intervalVector::iterator i = intervals.begin(); i != intervals.end(); ++i) {
        interval &interval = *i;
        if (interval.stop >= start && interval.start <= stop) {
          overlapping.push_back(std::move(interval.value->clone()));
        }
      }
    }

    if (left && start <= center) {
      left->findOverlapping(start, stop, overlapping);
    }

    if (right && stop >= center) {
      right->findOverlapping(start, stop, overlapping);
    }
  }

  bool hasOverlap(K start, K stop)
  {
    if (!intervals.empty() && ! (stop < intervals.front().start)) {
      for (typename intervalVector::iterator i = intervals.begin(); i != intervals.end(); ++i) {
        interval &interval = *i;
        if (interval.stop >= start && interval.start <= stop) {
          return true;
        }
      }
    }

    if (left && start <= center) {
      return left->hasOverlap(start, stop);
    }

    if (right && stop >= center) {
      return right->findOverlapping(start, stop);
    }

    return false;
  }

  void findContained(K start, K stop, overlapVector &contained)
  {
    if (!intervals.empty() && ! (stop < intervals.front().start)) {
      for (typename intervalVector::iterator i = intervals.begin(); i != intervals.end(); ++i) {
        interval &interval = *i;
        if (interval.start >= start && interval.stop <= stop) {
          contained.push_back(std::move(interval.value->clone()));
        }
      }
    }

    if (left && start <= center) {
      left->findContained(start, stop, contained);
    }

    if (right && stop >= center) {
      right->findContained(start, stop, contained);
    }
  }

  ~IntervalTree(void)
  {
    // traverse the left and right
    // delete them all the way down
    if (left) {
      delete left;
    }
    if (right) {
      delete right;
    }
  }

};

#endif