import context
import os

from koheron import load_instrument
from drivers.test_memory import TestMemory

host = os.getenv('HOST','192.168.1.100')
project = os.getenv('NAME','')
client = load_instrument(host, project)

test_memory = TestMemory(client)

def test_write_read_u32():
    assert(test_memory.write_read_u32())