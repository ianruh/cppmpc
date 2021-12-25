import unittest
import symengine
import cppmpc


class BasicSwigWrapperTests(unittest.TestCase):
    def test_echo(self):
        """
        Just test that we can convert to and from RCP<const Basic> without
        crashing and with equality still working.
        """
        x = symengine.Symbol("x")
        y = symengine.Symbol("y")

        self.assertEqual(x, cppmpc.echo(x))
        self.assertEqual(x * y, cppmpc.echo(x * y))


if __name__ == "__main__":
    unittest.main()
