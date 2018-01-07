from rmtest import ModuleTestCase
from redis import ResponseError
import unittest

class OMTestCase(ModuleTestCase('../src/orderedset.so')):

	def test_add_head(self):

		c, s = self.client, self.server

		self.assertEquals(1, self.cmd('os.addhead', 'test', 'foo'))
		self.assertEquals(2, self.cmd('os.addhead', 'test', 'bar!', 'baz'))
		self.assertEquals(3, self.cmd('os.card', 'test'))
		self.assertEquals(['bar!','baz','foo'], self.cmd('os.members', 'test'))
		self.assertEquals(1, self.cmd('os.addhead', 'test', 'foo', 'qux'))
		self.assertEquals(['foo','qux','bar!','baz'], self.cmd('os.members', 'test'))

	def test_add_tail(self):

		c, s = self.client, self.server

		self.assertEquals(1, self.cmd('os.addtail', 'test', 'foo'))
		self.assertEquals(2, self.cmd('os.addtail', 'test', 'bar!', 'baz'))
		self.assertEquals(3, self.cmd('os.card', 'test'))
		self.assertEquals(['foo','bar!','baz'], self.cmd('os.members', 'test'))
		self.assertEquals(1, self.cmd('os.addtail', 'test', 'foo', 'qux'))
		self.assertEquals(['bar!','baz','foo','qux'], self.cmd('os.members', 'test'))

	def test_add_after(self):

		c, s = self.client, self.server

		self.cmd('os.addhead', 'test', 'foo')
		self.assertEquals(1, self.cmd('os.addafter', 'test', 'foo', 'bar!'))
		self.assertEquals(2, self.cmd('os.card', 'test'))
		self.assertEquals(['foo','bar!'], self.cmd('os.members', 'test'))
		self.assertEquals(2, self.cmd('os.addafter', 'test', 'bar!', 'baz', 'baz', 'bar!', 'qux'))
		self.assertEquals(['foo','baz','bar!','qux'], self.cmd('os.members', 'test'))
		self.assertEquals(None, self.cmd('os.addafter', 'test', 'notexist', 'new'))
		self.assertEquals(0, self.cmd('os.addafter', 'test', 'qux', 'foo'))
		self.assertEquals(['baz','bar!','qux','foo'], self.cmd('os.members', 'test'))
		self.assertEquals(4, self.cmd('os.card', 'test'))

	def test_add_before(self):

		c, s = self.client, self.server

		self.cmd('os.addtail', 'test', 'foo')
		self.assertEquals(1, self.cmd('os.addbefore', 'test', 'foo', 'bar!'))
		self.assertEquals(2, self.cmd('os.card', 'test'))
		self.assertEquals(['bar!','foo'], self.cmd('os.members', 'test'))
		self.assertEquals(2, self.cmd('os.addbefore', 'test', 'foo', 'baz', 'baz', 'foo', 'qux'))
		self.assertEquals(['bar!','baz','foo','qux'], self.cmd('os.members', 'test'))
		self.assertEquals(None, self.cmd('os.addbefore', 'test', 'notexist', 'new'))
		self.assertEquals(0, self.cmd('os.addbefore', 'test', 'bar!', 'qux'))
		self.assertEquals(['qux','bar!','baz','foo'], self.cmd('os.members', 'test'))
		self.assertEquals(4, self.cmd('os.card', 'test'))

	def test_remove(self):

		c, s = self.client, self.server

		self.cmd('os.addhead', 'test', 'foo', 'bar!', 'baz', 'qux')
		self.assertEquals(1, self.cmd('os.rem', 'test', 'bar!'))
		self.assertEquals(3, self.cmd('os.card', 'test'))
		self.assertEquals(2, self.cmd('os.rem', 'test', 'foo', 'baz', 'notexist'))
		self.assertEquals(1, self.cmd('os.card', 'test'))
		self.assertEquals(1, self.cmd('os.rem', 'test', 'qux', 'bar!'))
		self.assertEquals(0, self.cmd('os.card', 'test'))

	def test_compare(self):

		c, s = self.client, self.server

		self.cmd('os.addhead', 'test', 'foo', 'bar!', 'baz')
		self.assertEquals(-1, self.cmd('os.compare', 'test', 'foo', 'bar!'))
		self.assertEquals(0, self.cmd('os.compare', 'test', 'bar!', 'bar!'))
		self.assertEquals(1, self.cmd('os.compare', 'test', 'baz', 'bar!'))
		self.assertEquals(None, self.cmd('os.compare', 'test', 'notexist', 'bar!'))

		for _ in c.retry_with_rdb_reload():
			self.assertEquals(-1, self.cmd('os.compare', 'test', 'foo', 'bar!'))
			self.assertEquals(0, self.cmd('os.compare', 'test', 'bar!', 'bar!'))
			self.assertEquals(1, self.cmd('os.compare', 'test', 'baz', 'bar!'))
			self.assertEquals(None, self.cmd('os.compare', 'test', 'notexist', 'bar!'))

	def test_next(self):

		c, s = self.client, self.server

		self.cmd('os.addhead', 'test', 'foo', 'bar!', 'baz', 'qux')
		self.assertEquals(['bar!','baz','qux'], self.cmd('os.next', 'test', 'foo', 3))
		self.assertEquals(['bar!','baz','qux'], self.cmd('os.next', 'test', 'foo', 10))
		self.assertEquals(['baz','qux'], self.cmd('os.next', 'test', 'bar!', 0))
		self.assertEquals(['qux'], self.cmd('os.next', 'test', 'baz', 2))
		self.assertEquals([], self.cmd('os.next', 'test', 'notexist', 1))
		self.assertRaises(ResponseError, self.cmd, 'os.next', 'test', 'baz', 'invalid')

		for _ in c.retry_with_rdb_reload():
			self.assertEquals(['bar!','baz','qux'], self.cmd('os.next', 'test', 'foo', 3))
			self.assertEquals(['qux'], self.cmd('os.next', 'test', 'baz', 2))
			self.assertEquals(['baz','qux'], self.cmd('os.next', 'test', 'bar!', 0))

	def test_prev(self):

		c, s = self.client, self.server

		self.cmd('os.addhead', 'test', 'foo', 'bar!', 'baz', 'qux')
		self.assertEquals(['baz','bar!','foo'], self.cmd('os.prev', 'test', 'qux', 3))
		self.assertEquals(['baz','bar!','foo'], self.cmd('os.prev', 'test', 'qux', 10))
		self.assertEquals(['bar!','foo'], self.cmd('os.prev', 'test', 'baz', 0))
		self.assertEquals(['foo'], self.cmd('os.prev', 'test', 'bar!', 2))
		self.assertEquals([], self.cmd('os.prev', 'test', 'notexist', 1))
		self.assertRaises(ResponseError, self.cmd, 'os.next', 'test', 'baz', 'invalid')

		for _ in c.retry_with_rdb_reload():
			self.assertEquals(['baz','bar!','foo'], self.cmd('os.prev', 'test', 'qux', 3))
			self.assertEquals(['foo'], self.cmd('os.prev', 'test', 'bar!', 2))
			self.assertEquals(['bar!','foo'], self.cmd('os.prev', 'test', 'baz', 0))

	def test_head(self):

		c, s = self.client, self.server

		self.cmd('os.addhead', 'test', 'foo', 'bar!', 'baz', 'qux')
		self.assertEquals(['foo','bar!','baz'], self.cmd('os.head', 'test', 3))
		self.assertEquals(['foo','bar!','baz', 'qux'], self.cmd('os.head', 'test', 10))
		self.assertEquals(['foo','bar!','baz', 'qux'], self.cmd('os.head', 'test', 0))
		self.assertRaises(ResponseError, self.cmd, 'os.head', 'test', 'invalid')

		for _ in c.retry_with_rdb_reload():
			self.assertEquals(['foo','bar!','baz'], self.cmd('os.head', 'test', 3))
			self.assertEquals(['foo','bar!','baz', 'qux'], self.cmd('os.head', 'test', 0))

	def test_tail(self):

		c, s = self.client, self.server

		self.cmd('os.addhead', 'test', 'foo', 'bar!', 'baz', 'qux')
		self.assertEquals(['qux','baz','bar!'], self.cmd('os.tail', 'test', 3))
		self.assertEquals(['qux','baz','bar!','foo'], self.cmd('os.tail', 'test', 10))
		self.assertEquals(['qux','baz','bar!','foo'], self.cmd('os.tail', 'test', 0))
		self.assertRaises(ResponseError, self.cmd, 'os.tail', 'test', 'invalid')

		for _ in c.retry_with_rdb_reload():
			self.assertEquals(['qux','baz','bar!'], self.cmd('os.tail', 'test', 3))
			self.assertEquals(['qux','baz','bar!','foo'], self.cmd('os.tail', 'test', 0))

if __name__ == "__main__":
    unittest.main()