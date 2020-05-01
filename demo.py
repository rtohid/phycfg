from __future__ import absolute_import

import numpy

__license__ = """
Copyright (c) 2020 R. Tohid

Distributed under the Boost Software License, Version 1.0. (See accompanying
file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
"""

from phyfleaux.api.directives import polyhedral
# from phyfleaux.api.algorithms import kmeans

# @polyhedral
# def matmul(N):
#     """Naive Matrix Multiplication."""

#     a = numpy.ones((N, N), dtype=int)
#     b = numpy.ones((N, N), dtype=int)
#     c = numpy.zeros((N, N), dtype=int)

#     for i in range(N):
#         for j in range(N):
#             for k in range(N):
#                 c[i][j] = c[i][j] + a[i][k] * b[k][j]
#     #     for j in range(N):
#     #             c[j] = a[i]
#     # for j in range(N):
#     #     c[j] = a[i]
#     #     for i in range(N):
#     #             c[i] = a[i]

#     return c


@polyhedral
def example_0(N, M):
    c = numpy.zeros((N, M), dtype=int)
    for i in range(N):
        for j in range(M):
            # c[i][j] = i * N + j
            c[i][j] = 42

    return c


print(example_0(4, 4))

# def test_kmeans(*args, **kwargs):
#     """Test all kmeans functionalities in scipy."""

#     pass
