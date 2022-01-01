import unittest
import symengine
import cppmpc


class OrderedSetTests(unittest.TestCase):
    def test_basic(self):
        x = symengine.Symbol("x")
        y = symengine.Symbol("y")
        z = symengine.Symbol("z")

        test_set = cppmpc.OrderedSet()

        test_set.append(x)
        self.assertTrue(test_set.isConsistent())
        test_set.append(y)
        self.assertTrue(test_set.isConsistent())
        test_set.append(z)
        self.assertTrue(test_set.isConsistent())

        self.assertEqual(x, test_set.at(0))
        self.assertEqual(y, test_set.at(1))
        self.assertEqual(z, test_set.at(2))

        self.assertEqual(test_set.size(), 3)

        test_set.remove(0)
        self.assertTrue(test_set.isConsistent())

        self.assertEqual(y, test_set.at(0))
        self.assertEqual(z, test_set.at(1))

    def test_union(self):
        x = symengine.Symbol("x")
        y = symengine.Symbol("y")
        z = symengine.Symbol("z")

        test_set1 = cppmpc.OrderedSet()
        test_set2 = cppmpc.OrderedSet()

        test_set1.append(x)
        test_set1.append(y)

        test_set1.append(y)
        test_set2.append(z)

        self.assertTrue(test_set1.isConsistent())
        self.assertTrue(test_set2.isConsistent())

        test_set1.unionWith(test_set2)

        self.assertTrue(test_set1.isConsistent())
        self.assertEqual(test_set1.size(), 3)

    def test_subset(self):
        x = symengine.Symbol("x")
        y = symengine.Symbol("y")
        z = symengine.Symbol("z")

        test_set1 = cppmpc.OrderedSet()
        test_set2 = cppmpc.OrderedSet()
        test_set3 = cppmpc.OrderedSet()

        test_set1.append(x)
        test_set1.append(y)

        test_set1.append(y)
        test_set2.append(z)

        test_set3.append(x)

        self.assertTrue(test_set1.isSubset(test_set3))
        self.assertFalse(test_set3.isSubset(test_set2))
        self.assertFalse(test_set1.isSubset(test_set2))


if __name__ == "__main__":
    unittest.main()
