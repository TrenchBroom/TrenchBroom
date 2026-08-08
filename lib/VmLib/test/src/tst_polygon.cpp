/*
 Copyright (C) 2010 Kristian Duske
 Copyright (C) 2015 Eric Wasylishen

 Permission is hereby granted, free of charge, to any person obtaining a copy of this
 software and associated documentation files (the "Software"), to deal in the Software
 without restriction, including without limitation the rights to use, copy, modify, merge,
 publish, distribute, sublicense, and/or sell copies of the Software, and to permit
 persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
 FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 DEALINGS IN THE SOFTWARE.
*/

#include "vm/approx.h"
#include "vm/mat.h"
#include "vm/mat_ext.h"
#include "vm/polygon.h"
#include "vm/vec_ext.h"
#include "vm/vec_io.h" // IWYU pragma: keep

#include <iterator>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

namespace vm
{
using namespace Catch::Matchers;

TEST_CASE("polygon")
{
  SECTION("constructor_default")
  {
    CHECK(polygon3d{}.vertices().size() == 0u);
  }

  SECTION("constructor_with_initializer_list")
  {
    CHECK(
      polygon3d{
        vec3d{+1, +1, 0},
        vec3d{+1, -1, 0},
        vec3d{-1, -1, 0},
        vec3d{-1, +1, 0},
      }
        .vertices()
      == std::vector<vec3d>{
        {-1, -1, 0},
        {-1, +1, 0},
        {+1, +1, 0},
        {+1, -1, 0},
      });
  }

  SECTION("construct_with_vertex_list")
  {
    const auto vertices = std::vector<vec3d>{
      {+1, +1, 0},
      {+1, -1, 0},
      {-1, -1, 0},
      {-1, +1, 0},
    };
    CHECK(
      polygon3d{vertices}.vertices()
      == std::vector<vec3d>{
        {-1, -1, 0},
        {-1, +1, 0},
        {+1, +1, 0},
        {+1, -1, 0},
      });
  }

  SECTION("has_vertex")
  {
    const auto vertices = std::vector<vec3d>{
      {+1, +1, 0},
      {+1, -1, 0},
      {-1, -1, 0},
      {-1, +1, 0},
    };

    const auto p = polygon3d{vertices};
    for (const auto& v : vertices)
    {
      CHECK(p.hasVertex(v));
    }

    CHECK(!(p.hasVertex(vec3d{1, 1, 1})));
  }

  SECTION("vertex_count")
  {
    const auto vertices = std::vector<vec3d>{
      {+1, +1, 0},
      {+1, -1, 0},
      {-1, -1, 0},
      {-1, +1, 0},
    };

    const auto p = polygon3d{vertices};
    CHECK(p.vertexCount() == 4u);
    CHECK(polygon3d{}.vertexCount() == 0u);
  }

  SECTION("vertices")
  {
    const auto vertices = std::vector<vec3d>{
      {-1, -1, 0},
      {-1, +1, 0},
      {+1, +1, 0},
      {+1, -1, 0},
    };

    const auto p = polygon3d{vertices};
    CHECK_THAT(p.vertices(), Equals(vertices));
  }

  SECTION("center")
  {
    const auto vertices = std::vector<vec3d>{
      {-1, -1, 0},
      {-1, +1, 0},
      {+1, +1, 0},
      {+1, -1, 0},
    };

    const auto p = polygon3d{vertices};
    CHECK(p.center() == approx(vec3d{0, 0, 0}));
  }

  SECTION("invert")
  {
    const auto p = polygon3d{
      vec3d{-1, -1, 0},
      vec3d{-1, +1, 0},
      vec3d{+1, +1, 0},
      vec3d{+1, -1, 0},
    };

    CHECK(
      p.invert().vertices()
      == std::vector<vec3d>{
        vec3d{-1, -1, 0},
        vec3d{+1, -1, 0},
        vec3d{+1, +1, 0},
        vec3d{-1, +1, 0},
      });
  }

  SECTION("translate")
  {
    const auto p = polygon3d{
      {+1, +1, 0},
      {+1, -1, 0},
      {-1, -1, 0},
      {-1, +1, 0},
    };

    const auto t = vec3d{1, 2, 3};
    CHECK(p.translate(t).vertices() == p.vertices() + t);
  }

  SECTION("transform")
  {
    const auto p = polygon3d{
      {+1, +1, 0},
      {+1, -1, 0},
      {-1, -1, 0},
      {-1, +1, 0},
    };

    const auto t = rotation_matrix(to_radians(14.0), to_radians(13.0), to_radians(44.0))
                   * translation_matrix(vec3d{1, 2, 3});
    CHECK(p.transform(t).vertices() == polygon3d{t * p.vertices()}.vertices());
  }

  SECTION("get_vertices")
  {
    const auto p1 = polygon3d{
      {+1, +1, 0},
      {+1, -1, 0},
      {-1, -1, 0},
      {-1, +1, 0},
    };
    const auto p2 = p1.translate(vec3d{1, 2, 3});
    const auto ps = std::vector<polygon3d>{p1, p2};

    auto exp = p1.vertices();
    exp.insert(std::end(exp), p2.vertices().begin(), p2.vertices().end());

    auto act = std::vector<vec3d>();
    polygon3d::get_vertices(std::begin(ps), std::end(ps), std::back_inserter(act));

    CHECK(act == exp);
  }

  SECTION("compare")
  {
    CHECK(compare(polygon3d{}, polygon3d{}) == std::strong_ordering::equal);

    CHECK(
      compare(
        polygon3d{
          {-1, -1, 0},
          {-1, +1, 0},
          {+1, +1, 0},
          {+1, -1, 0},
        },
        polygon3d{
          {-1, -1, 0},
          {-1, +1, 0},
          {+1, +1, 0},
          {+1, -1, 0},
        })
      == std::strong_ordering::equal);

    CHECK(
      compare(
        polygon3d{
          {-1, -1, 0},
          {-1, +1, 0},
          {+1, +1, 0},
          {+1, -1, 0},
        },
        polygon3d{
          {-2, -1, 0},
          {-1, +1, 0},
          {+1, +1, 0},
          {+1, -1, 0},
        },
        2.0)
      == std::strong_ordering::equal);

    CHECK(
      compare(
        polygon3d{
          {-1, -1, 0},
          {-1, +1, 0},
          {+1, +1, 0},
        },
        polygon3d{
          {-1, -1, 0},
          {-1, +1, 0},
          {+1, +1, 0},
          {+1, -1, 0},
        })
      == std::strong_ordering::less);

    CHECK(
      compare(
        polygon3d{
          {-1, -1, 0},
          {-1, +1, 0},
          {+1, +1, 0},
          {+1, -1, 0},
        },
        polygon3d{
          {-1, -1, 0},
          {-1, +1, 0},
          {+1, +1, 0},
        })
      == std::strong_ordering::greater);

    CHECK(
      compare(
        polygon3d{
          {-1, -1, 0},
          {-1, +1, 0},
          {+1, +1, 0},
        },
        polygon3d{
          {+1, -1, 0},
          {-1, +1, 0},
          {+1, +1, 0},
          {+1, -1, 0},
        })
      == std::strong_ordering::less);

    CHECK(
      compare(
        polygon3d{
          {-1, -1, 0},
          {-1, +1, 0},
          {+1, +1, 0},
        },
        polygon3d{
          {+1, -1, 0},
          {-1, +1, 0},
        })
      == std::strong_ordering::less);

    CHECK(
      compare(
        polygon3d{
          {+1, -1, 0},
          {-1, +1, 0},
          {+1, +1, 0},
          {+1, -1, 0},
        },
        polygon3d{
          {-1, -1, 0},
          {-1, +1, 0},
          {+1, +1, 0},
        })
      == std::strong_ordering::greater);

    CHECK(
      compare(
        polygon3d{
          {+1, -1, 0},
          {-1, +1, 0},
        },
        polygon3d{
          {-1, -1, 0},
          {-1, +1, 0},
          {+1, +1, 0},
        })
      == std::strong_ordering::greater);
  }

  SECTION("operator_equal")
  {
    CHECK(polygon3d{} == polygon3d{});

    CHECK(
      polygon3d{
        {-1, -1, 0},
        {-1, +1, 0},
        {+1, +1, 0},
        {+1, -1, 0},
      }
      == polygon3d{
        {-1, -1, 0},
        {-1, +1, 0},
        {+1, +1, 0},
        {+1, -1, 0},
      });

    CHECK_FALSE(
      polygon3d{
        {-1, -1, 0},
        {-1, +1, 0},
        {+1, +1, 0},
      }
      == polygon3d{
        {-1, -1, 0},
        {-1, +1, 0},
        {+1, +1, 0},
        {+1, -1, 0},
      });

    CHECK_FALSE(
      polygon3d{
        {-1, -1, 0},
        {-1, +1, 0},
        {+1, +1, 0},
        {+1, -1, 0},
      }
      == polygon3d{
        {-1, -1, 0},
        {-1, +1, 0},
        {+1, +1, 0},
      });

    CHECK_FALSE(
      polygon3d{
        {-1, -1, 0},
        {-1, +1, 0},
        {+1, +1, 0},
      }
      == polygon3d{
        {+1, -1, 0},
        {-1, +1, 0},
        {+1, +1, 0},
        {+1, -1, 0},
      });

    CHECK_FALSE(
      polygon3d{
        {-1, -1, 0},
        {-1, +1, 0},
        {+1, +1, 0},
      }
      == polygon3d{
        {+1, -1, 0},
        {-1, +1, 0},
      });

    CHECK_FALSE(
      polygon3d{
        {+1, -1, 0},
        {-1, +1, 0},
        {+1, +1, 0},
        {+1, -1, 0},
      }
      == polygon3d{
        {-1, -1, 0},
        {-1, +1, 0},
        {+1, +1, 0},
      });

    CHECK_FALSE(
      polygon3d{
        {+1, -1, 0},
        {-1, +1, 0},
      }
      == polygon3d{
        {-1, -1, 0},
        {-1, +1, 0},
        {+1, +1, 0},
      });
  }

  SECTION("operator_not_equal")
  {
    CHECK(!(polygon3d{} != polygon3d{}));

    CHECK_FALSE(
      polygon3d{
        {-1, -1, 0},
        {-1, +1, 0},
        {+1, +1, 0},
        {+1, -1, 0},
      }
      != polygon3d{
        {-1, -1, 0},
        {-1, +1, 0},
        {+1, +1, 0},
        {+1, -1, 0},
      });

    CHECK(
      polygon3d{
        {-1, -1, 0},
        {-1, +1, 0},
        {+1, +1, 0},
      }
      != polygon3d{
        {-1, -1, 0},
        {-1, +1, 0},
        {+1, +1, 0},
        {+1, -1, 0},
      });

    CHECK(
      polygon3d{
        {-1, -1, 0},
        {-1, +1, 0},
        {+1, +1, 0},
        {+1, -1, 0},
      }
      != polygon3d{
        {-1, -1, 0},
        {-1, +1, 0},
        {+1, +1, 0},
      });

    CHECK(
      polygon3d{
        {-1, -1, 0},
        {-1, +1, 0},
        {+1, +1, 0},
      }
      != polygon3d{
        {+1, -1, 0},
        {-1, +1, 0},
        {+1, +1, 0},
        {+1, -1, 0},
      });

    CHECK(
      polygon3d{
        {-1, -1, 0},
        {-1, +1, 0},
        {+1, +1, 0},
      }
      != polygon3d{
        {+1, -1, 0},
        {-1, +1, 0},
      });

    CHECK(
      polygon3d{
        {+1, -1, 0},
        {-1, +1, 0},
        {+1, +1, 0},
        {+1, -1, 0},
      }
      != polygon3d{
        {-1, -1, 0},
        {-1, +1, 0},
        {+1, +1, 0},
      });

    CHECK(
      polygon3d{
        {+1, -1, 0},
        {-1, +1, 0},
      }
      != polygon3d{
        {-1, -1, 0},
        {-1, +1, 0},
        {+1, +1, 0},
      });
  }

  SECTION("operator_less_than")
  {
    CHECK_FALSE(
      polygon3d{
        {-1, -1, 0},
        {-1, +1, 0},
        {+1, +1, 0},
        {+1, -1, 0},
      }
      < polygon3d{
        {-1, -1, 0},
        {-1, +1, 0},
        {+1, +1, 0},
        {+1, -1, 0},
      });

    CHECK(
      polygon3d{
        {-1, -1, 0},
        {-1, +1, 0},
        {+1, +1, 0},
      }
      < polygon3d{
        {-1, -1, 0},
        {-1, +1, 0},
        {+1, +1, 0},
        {+1, -1, 0},
      });

    CHECK_FALSE(
      polygon3d{
        {-1, -1, 0},
        {-1, +1, 0},
        {+1, +1, 0},
        {+1, -1, 0},
      }
      < polygon3d{
        {-1, -1, 0},
        {-1, +1, 0},
        {+1, +1, 0},
      });
  }

  SECTION("operator_less_than_or_equal")
  {
    CHECK(
      polygon3d{
        {-1, -1, 0},
        {-1, +1, 0},
        {+1, +1, 0},
        {+1, -1, 0},
      }
      <= polygon3d{
        {-1, -1, 0},
        {-1, +1, 0},
        {+1, +1, 0},
        {+1, -1, 0},
      });

    CHECK(
      polygon3d{
        {-1, -1, 0},
        {-1, +1, 0},
        {+1, +1, 0},
      }
      <= polygon3d{
        {-1, -1, 0},
        {-1, +1, 0},
        {+1, +1, 0},
        {+1, -1, 0},
      });

    CHECK_FALSE(
      polygon3d{
        {-1, -1, 0},
        {-1, +1, 0},
        {+1, +1, 0},
        {+1, -1, 0},
      }
      <= polygon3d{
        {-1, -1, 0},
        {-1, +1, 0},
        {+1, +1, 0},
      });
  }

  SECTION("operator_greater_than")
  {
    CHECK_FALSE(
      polygon3d{
        {-1, -1, 0},
        {-1, +1, 0},
        {+1, +1, 0},
        {+1, -1, 0},
      }
      > polygon3d{
        {-1, -1, 0},
        {-1, +1, 0},
        {+1, +1, 0},
        {+1, -1, 0},
      });

    CHECK_FALSE(
      polygon3d{
        {-1, -1, 0},
        {-1, +1, 0},
        {+1, +1, 0},
      }
      > polygon3d{
        {-1, -1, 0},
        {-1, +1, 0},
        {+1, +1, 0},
        {+1, -1, 0},
      });

    CHECK(
      polygon3d{
        {-1, -1, 0},
        {-1, +1, 0},
        {+1, +1, 0},
        {+1, -1, 0},
      }
      > polygon3d{
        {-1, -1, 0},
        {-1, +1, 0},
        {+1, +1, 0},
      });
  }

  SECTION("operator_greater_than_or_equal")
  {
    CHECK(
      polygon3d{
        {-1, -1, 0},
        {-1, +1, 0},
        {+1, +1, 0},
        {+1, -1, 0},
      }
      >= polygon3d{
        {-1, -1, 0},
        {-1, +1, 0},
        {+1, +1, 0},
        {+1, -1, 0},
      });

    CHECK_FALSE(
      polygon3d{
        {-1, -1, 0},
        {-1, +1, 0},
        {+1, +1, 0},
      }
      >= polygon3d{
        {-1, -1, 0},
        {-1, +1, 0},
        {+1, +1, 0},
        {+1, -1, 0},
      });

    CHECK(
      polygon3d{
        {-1, -1, 0},
        {-1, +1, 0},
        {+1, +1, 0},
        {+1, -1, 0},
      }
      >= polygon3d{
        {-1, -1, 0},
        {-1, +1, 0},
        {+1, +1, 0},
      });
  }

  SECTION("compare_unoriented_empty_polygon")
  {
    const auto p1 = polygon3d{};
    CHECK(compareUnoriented(p1, polygon3d{}) == std::strong_ordering::equal);
    CHECK(compareUnoriented(p1, polygon3d{{0, 0, 0}}) == std::strong_ordering::less);

    const auto p2 = polygon3d{{0, 0, 0}};
    CHECK(compareUnoriented(p2, p1) == std::strong_ordering::greater);
    CHECK(compareUnoriented(p2, polygon3d{{0, 0, 0}}) == std::strong_ordering::equal);
  }

  SECTION("testBackwardComparePolygonWithOneVertex")
  {
    const auto p2 = polygon3d{{0, 0, 0}};
    CHECK(compareUnoriented(p2, polygon3d{{0, 0, 0}}) == std::strong_ordering::equal);
    CHECK(
      compareUnoriented(p2, polygon3d{{0, 0, 0}, {0, 0, 0}})
      == std::strong_ordering::less);
  }

  SECTION("compare_unoriented")
  {
    const auto p1 = polygon3d{
      {-1.0, -1.0, 0.0},
      {+1.0, -1.0, 0.0},
      {+1.0, +1.0, 0.0},
      {-1.0, +1.0, 0.0},
    };
    const auto p2 = polygon3d{
      {-1.0, +1.0, 0.0},
      {+1.0, +1.0, 0.0},
      {+1.0, -1.0, 0.0},
      {-1.0, -1.0, 0.0},
    };
    CHECK(compareUnoriented(p1, p1) == std::strong_ordering::equal);
    CHECK(compareUnoriented(p1, p2) == std::strong_ordering::equal);
    CHECK(compareUnoriented(p2, p1) == std::strong_ordering::equal);
    CHECK(compareUnoriented(p2, p2) == std::strong_ordering::equal);
  }
}
} // namespace vm
