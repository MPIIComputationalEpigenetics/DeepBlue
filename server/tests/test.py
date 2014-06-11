import sys
from unittest import TestLoader, TextTestRunner, TestSuite

import helpers
import test_cases

def test(names):
  runner = TextTestRunner(verbosity=2)
  loader = TestLoader()
  tests = []

  for name in names:
    test_file = getattr(test_cases, name)
    for test_case in dir(test_file):
      if test_case.startswith("Test"):
        tests += loader.loadTestsFromTestCase(getattr(test_file, test_case))

  suite = TestSuite(tuple(tests))
  runner.run(suite)


def test_all():
  test([t for t in dir(test_cases) if t[0] != "_"])


if __name__ == "__main__":
  epidb_process = helpers.setUp()
  try:
    if len(sys.argv) == 1:
      test_all()
    else:
      test(sys.argv[1:])
  finally:
    helpers.tearDown(epidb_process)
