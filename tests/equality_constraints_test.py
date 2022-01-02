import unittest
import symengine
import cppmpc
from sympy import Rational

def to_sympy(arg):
    return arg._sympy_()

class EqualityConstraintsTests(unittest.TestCase):

    def test_matrix_construction(self):
        x = symengine.Symbol("$v_x")
        y = symengine.Symbol("$v_y")
        z = symengine.Symbol("$v_z")
        
        a = symengine.Symbol("$p_a")
        
        ordering = cppmpc.OrderedSet()
        ordering.append(z)
        ordering.append(y)
        ordering.append(x)

        constraints = cppmpc.SymbolicEqualityConstraints()
        constraints.appendConstraint(symengine.Eq(x, 3*y + 4))
        constraints.appendConstraint(symengine.Eq((z+a)/2, 7))

        system = constraints.convertToLinearSystem(ordering)

        expected_mat = symengine.DenseMatrix(row=2, col=3, v=[
            0, 3, -1, Rational(1/2), 0, 0
        ])

        expected_const = symengine.DenseMatrix(row=2, col=1, v=[
            -4, 7 + Rational(-1/2)*a
        ])


        self.assertEqual(to_sympy(system[0]), expected_mat)
        self.assertEqual(to_sympy(system[1]), expected_const)

if __name__ == "__main__":
    unittest.main()
