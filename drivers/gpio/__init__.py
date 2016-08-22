# -*- coding: utf-8 -*-

from koheron import command

class Gpio(object):

    def __init__(self, client):
        self.client = client

    @command('GPIO', 'II')
    def set_data(self, channel, value): pass

    @command('GPIO', 'I')
    def get_data(self, channel):
        return self.client.recv_uint32()

    @command('GPIO', 'II')
    def set_bit(self, index, channel): pass

    @command('GPIO', 'II')
    def clear_bit(self, index, channel): pass

    @command('GPIO', 'II')
    def toggle_bit(self, index, channel): pass

    @command('GPIO', 'II')
    def read_bit(self, index, channel):
        return self.client.recv_bool()

    @command('GPIO', 'II')
    def set_as_input(self, index, channel): pass

    @command('GPIO', 'II')
    def set_as_output(self, index, channel): pass
