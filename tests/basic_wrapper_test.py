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

    def test_echo_nested(self):
        """
        Just test that we can convert to and from RCP<const Basic> without
        crashing and with equality still working.
        """
        x = symengine.Symbol("x")
        y = symengine.Symbol("y")
        z = symengine.Symbol("z")

        expr1 = x * y
        expr2 = (x * y) + z
        self.assertEqual(expr1, cppmpc.echo(expr1))
        self.assertEqual(expr2, cppmpc.echo(expr2))


if __name__ == "__main__":
    unittest.main()
