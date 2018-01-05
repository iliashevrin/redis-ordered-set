from rmtest import ModuleTestCase
from redis import ResponseError
import unittest

class OMTestCase(ModuleTestCase('../src/orderedset.so')):

	def test_add_first(self):

		c, s = self.client, self.server

		self.assertOk(self.cmd('om.addfirst', 'test', 'foo'))
		self.assertOk(self.cmd('om.addfirst', 'test', 'bar!'))
		self.assertEquals(2, self.cmd('om.card', 'test'))
		self.assertEquals(['bar!','foo'], self.cmd('om.list', 'test'))

	def test_add_last(self):

		c, s = self.client, self.server

		self.assertOk(self.cmd('om.addlast', 'test', 'foo'))
		self.assertOk(self.cmd('om.addlast', 'test', 'bar!'))
		self.assertEquals(2, self.cmd('om.card', 'test'))
		self.assertEquals(['foo','bar!'], self.cmd('om.list', 'test'))

	def test_add_after(self):

		c, s = self.client, self.server

		self.cmd('om.addfirst', 'test', 'foo')
		self.assertOk(self.cmd('om.addafter', 'test', 'foo', 'bar!'))
		self.assertEquals(None, self.cmd('om.addafter', 'test2', 'foo', 'bar!'))
		self.assertEquals(2, self.cmd('om.card', 'test'))
		self.assertEquals(['foo','bar!'], self.cmd('om.list', 'test'))
		self.assertOk(self.cmd('om.addafter', 'test', 'bar!', 'bar!'))
		self.assertEquals(['foo','bar!'], self.cmd('om.list', 'test'))
		self.assertRaises(ResponseError, self.cmd, 'om.addafter', 'test', 'notexist', 'new')
		self.assertOk(self.cmd('om.addafter', 'test', 'bar!', 'baz'))
		self.assertOk(self.cmd('om.addafter', 'test', 'baz', 'foo'))
		self.assertEquals(3, self.cmd('om.card', 'test'))
		self.assertEquals(['bar!','baz','foo'], self.cmd('om.list', 'test'))

	def test_add_before(self):

		c, s = self.client, self.server

		self.cmd('om.addlast', 'test', 'foo')
		self.assertOk(self.cmd('om.addbefore', 'test', 'foo', 'bar!'))
		self.assertRaises(None, self.cmd('om.addbefore', 'test2', 'foo', 'bar!'))
		self.assertEquals(2, self.cmd('om.card', 'test'))
		self.assertEquals(['bar!','foo'], self.cmd('om.list', 'test'))
		self.assertOk(self.cmd('om.addbefore', 'test', 'bar!', 'bar!'))
		self.assertEquals(['bar!','foo'], self.cmd('om.list', 'test'))
		self.assertRaises(ResponseError, self.cmd, 'om.addbefore', 'test', 'notexist', 'new')
		self.assertOk(self.cmd('om.addbefore', 'test', 'bar!', 'baz'))
		self.assertOk(self.cmd('om.addbefore', 'test', 'baz', 'foo'))
		self.assertEquals(3, self.cmd('om.card', 'test'))
		self.assertEquals(['foo','baz','bar!'], self.cmd('om.list', 'test'))

	def test_remove(self):

		c, s = self.client, self.server

		self.cmd('om.addfirst', 'test', 'foo')
		self.cmd('om.addafter', 'test', 'foo', 'bar!')
		self.cmd('om.addafter', 'test', 'bar!', 'baz')
		self.assertOk(self.cmd('om.remove', 'test', 'bar!'))
		self.assertRaises(None, self.cmd('om.remove', 'test2', 'baz'))
		self.assertEquals(2, self.cmd('om.card', 'test'))
		self.assertRaises(ResponseError, self.cmd, 'om.remove', 'test', 'notexist')
		self.assertOk(self.cmd('om.remove', 'test', 'foo'))
		self.assertOk(self.cmd('om.remove', 'test', 'baz'))
		self.assertEquals(0, self.cmd('om.card', 'test'))

	def test_compare(self):

		c, s = self.client, self.server

		self.cmd('om.addfirst', 'test', 'foo')
		self.cmd('om.addafter', 'test', 'foo', 'bar!')
		self.cmd('om.addafter', 'test', 'bar!', 'baz')
		self.assertEquals(-1, self.cmd('om.compare', 'test', 'foo', 'bar!'))
		self.assertEquals(0, self.cmd('om.compare', 'test', 'bar!', 'bar!'))
		self.assertEquals(1, self.cmd('om.compare', 'test', 'baz', 'bar!'))
		self.assertEquals(None, self.cmd('om.compare', 'test2', 'foo', 'bar!'))
		self.assertRaises(ResponseError, self.cmd, 'om.compare', 'test', 'notexist', 'bar!')

		for _ in c.retry_with_rdb_reload():
			self.assertEquals(-1, self.cmd('om.compare', 'test', 'foo', 'bar!'))
			self.assertEquals(0, self.cmd('om.compare', 'test', 'bar!', 'bar!'))
			self.assertEquals(1, self.cmd('om.compare', 'test', 'baz', 'bar!'))
			self.assertRaises(ResponseError, self.cmd, 'om.compare', 'test', 'notexist', 'bar!')

	def test_next(self):

		c, s = self.client, self.server

		self.cmd('om.addfirst', 'test', 'foo')
		self.cmd('om.addafter', 'test', 'foo', 'bar!')
		self.cmd('om.addafter', 'test', 'bar!', 'baz')
		self.cmd('om.addafter', 'test', 'baz', 'qux')
		self.assertEquals(['bar!','baz','qux'], self.cmd('om.next', 'test', 'foo', 3))
		self.assertEquals(['bar!','baz','qux'], self.cmd('om.next', 'test', 'foo', 10))
		self.assertEquals(['qux'], self.cmd('om.next', 'test', 'baz', 2))
		self.assertEquals([], self.cmd('om.next', 'test2', 'baz', 2))
		self.assertRaises(ResponseError, self.cmd, 'om.next', 'test', 'notexist', 1)
		self.assertRaises(ResponseError, self.cmd, 'om.next', 'test', 'baz', 'invalid')

		for _ in c.retry_with_rdb_reload():
			self.assertEquals(['bar!','baz','qux'], self.cmd('om.next', 'test', 'foo', 3))
			self.assertEquals(['qux'], self.cmd('om.next', 'test', 'baz', 2))

	def test_prev(self):

		c, s = self.client, self.server

		self.cmd('om.addfirst', 'test', 'foo')
		self.cmd('om.addafter', 'test', 'foo', 'bar!')
		self.cmd('om.addafter', 'test', 'bar!', 'baz')
		self.cmd('om.addafter', 'test', 'baz', 'qux')
		self.assertEquals(['baz','bar!','foo'], self.cmd('om.prev', 'test', 'qux', 3))
		self.assertEquals(['baz','bar!','foo'], self.cmd('om.prev', 'test', 'qux', 10))
		self.assertEquals(['foo'], self.cmd('om.prev', 'test', 'bar!', 2))
		self.assertEquals([], self.cmd('om.prev', 'test2', 'baz', 2))
		self.assertRaises(ResponseError, self.cmd, 'om.prev', 'test', 'notexist', 1)
		self.assertRaises(ResponseError, self.cmd, 'om.prev', 'test', 'baz', 'invalid')

		for _ in c.retry_with_rdb_reload():
			self.assertEquals(['baz','bar!','foo'], self.cmd('om.prev', 'test', 'qux', 3))
			self.assertEquals(['foo'], self.cmd('om.prev', 'test', 'bar!', 2))

	def test_first(self):

		c, s = self.client, self.server

		self.cmd('om.addfirst', 'test', 'foo')
		self.cmd('om.addafter', 'test', 'foo', 'bar!')
		self.cmd('om.addafter', 'test', 'bar!', 'baz')
		self.cmd('om.addafter', 'test', 'baz', 'qux')
		self.assertEquals(['foo','bar!','baz'], self.cmd('om.first', 'test', 3))
		self.assertEquals(['foo','bar!','baz', 'qux'], self.cmd('om.first', 'test', 10))
		self.assertEquals([], self.cmd('om.first', 'test2', 2))
		self.assertRaises(ResponseError, self.cmd, 'om.first', 'test', 'invalid')

		for _ in c.retry_with_rdb_reload():
			self.assertEquals(['foo','bar!','baz'], self.cmd('om.first', 'test', 3))

	def test_last(self):

		c, s = self.client, self.server

		self.cmd('om.addfirst', 'test', 'foo')
		self.cmd('om.addafter', 'test', 'foo', 'bar!')
		self.cmd('om.addafter', 'test', 'bar!', 'baz')
		self.cmd('om.addafter', 'test', 'baz', 'qux')
		self.assertEquals(['qux','baz','bar!'], self.cmd('om.last', 'test', 3))
		self.assertEquals(['qux','baz','bar!','foo'], self.cmd('om.last', 'test', 10))
		self.assertEquals([], self.cmd('om.last', 'test2', 2))
		self.assertRaises(ResponseError, self.cmd, 'om.last', 'test', 'invalid')

		for _ in c.retry_with_rdb_reload():
			self.assertEquals(['qux','baz','bar!'], self.cmd('om.last', 'test', 3))

if __name__ == "__main__":
    unittest.main()