import queue
import socket
import time
import unittest
from network import Connection
from protocol import LineDecoder, parse_sample
from view3d import rotate


class ProtocolTests(unittest.TestCase):
    def test_fragmented_and_grouped(self):
        d = LineDecoder()
        self.assertEqual(d.feed(b'1;2'), [])
        self.assertEqual(d.feed(b';3\r\n12:00:00\nIMU;1;2;3;4;5;6\n'), ['1;2;3','12:00:00','IMU;1;2;3;4;5;6'])
        self.assertEqual(parse_sample('1;2;3').az,3)
        self.assertEqual(parse_sample('IMU;1;2;3;4;5;6').yaw,6)

    def test_invalid(self):
        for text in ('12:00:00','nan;2;3','inf;2;3','1;2','1;2;3;4;5','IMU;1;2;3','1;2;1e99','hola'):
            self.assertIsNone(parse_sample(text),text)
        with self.assertRaises(ValueError):
            LineDecoder(4).feed(b'12345')
        with self.assertRaises(ValueError):
            LineDecoder(4).feed(b'12345\n')

    def test_real_esp32_frame(self):
        # Trama visible en la captura: rumbo;roll;pitch;accX;accY;accZ.
        s = parse_sample('359.437;5.125;0.312;-0.010;0.040;-0.030')
        self.assertEqual((s.ax, s.ay, s.az), (-0.010, 0.040, -0.030))
        self.assertEqual((s.roll, s.pitch, s.yaw), (5.125, 0.312, 359.437))
        self.assertIsNone(parse_sample('359.437;5.125;nan;0;0;0'))

    def test_rotation(self):
        x,y,z=rotate((1,0,0),0,0,90)
        self.assertAlmostEqual(x,0)
        self.assertAlmostEqual(y,1)
        self.assertAlmostEqual(z,0)
        self.assertAlmostEqual(parse_sample('0;0;9.81').magnitude,9.81)


class SocketTests(unittest.TestCase):
    def wait_event(self, conn, kind, fragment):
        deadline=time.monotonic()+3
        while time.monotonic()<deadline:
            try:
                k,v=conn.events.get(timeout=.1)
            except queue.Empty:
                continue
            if k==kind and fragment in v:
                return v
        self.fail(f'No llegó {kind}: {fragment}')

    def test_server_and_reconnect(self):
        conn=Connection()
        with socket.socket() as probe:
            probe.bind(('127.0.0.1',0))
            port=probe.getsockname()[1]
        try:
            conn.start('Servidor','127.0.0.1',port)
            self.wait_event(conn,'status','Esperando')
            for _ in range(2):
                with socket.create_connection(('127.0.0.1',port),timeout=1) as client:
                    client.sendall(b'359.437;5.125;')
                    client.sendall(b'0.312;-0.010;0.040;-0.030\n08:30:00\n')
                    line = self.wait_event(conn,'line','359.437;')
                    self.assertEqual(parse_sample(line).yaw, 359.437)
                    self.wait_event(conn,'line','08:30:00')
                self.wait_event(conn,'status','Esperando')
        finally:
            conn.stop()
        self.assertIsNone(conn.thread)

    def test_client_and_stop(self):
        conn=Connection()
        with socket.socket() as listener:
            listener.bind(('127.0.0.1',0))
            listener.listen()
            listener.settimeout(2)
            try:
                conn.start('Cliente','127.0.0.1',listener.getsockname()[1])
                sock,_=listener.accept()
                with sock:
                    sock.sendall(b'IMU;0;0;9.81;10;20;30\n')
                    self.wait_event(conn,'line','IMU;')
                    conn.stop()
            finally:
                conn.stop()


if __name__=='__main__':
    unittest.main()
