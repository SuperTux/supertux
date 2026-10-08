import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch
sys.path.insert(0,str(Path(__file__).parents[2]/'tools/web'))
from upload_assets import upload
from package_assets import digest


class UploadTests(unittest.TestCase):
    def test_verified_existing_payload_is_not_uploaded(self):
        with tempfile.TemporaryDirectory() as directory:
            path=Path(directory)/'payload';path.write_bytes(b'correct')
            head=subprocess.CompletedProcess([],0,json.dumps({'ContentLength':7,'Metadata':{'sha256':digest(path)}}),'')
            with patch('upload_assets.subprocess.run',return_value=head) as run:
                self.assertTrue(upload('key',(path,'audio/ogg'),'https://r2.example','bucket').startswith('present:'))
                self.assertEqual(run.call_count,1)

    def test_legacy_object_requires_actual_byte_verification(self):
        with tempfile.TemporaryDirectory() as directory:
            path=Path(directory)/'payload';path.write_bytes(b'correct')
            for contents in (b'correct',b'damaged'):
                calls=[]
                def aws(arguments,**kwargs):
                    calls.append(arguments)
                    if arguments[1]=='s3api': return subprocess.CompletedProcess([],0,json.dumps({'ContentLength':7}), '')
                    Path(arguments[4]).write_bytes(contents)
                    return subprocess.CompletedProcess([],0,b'',b'')
                with patch('upload_assets.subprocess.run',side_effect=aws):
                    if contents==b'correct': self.assertTrue(upload('key',(path,'audio/ogg'),'https://r2.example','bucket').startswith('present:'))
                    else:
                        with self.assertRaises(RuntimeError): upload('key',(path,'audio/ogg'),'https://r2.example','bucket')
                    self.assertEqual(len(calls),2)
                    self.assertTrue(calls[1][3].startswith('s3://'))

    def test_access_denied_is_never_treated_as_missing(self):
        with tempfile.TemporaryDirectory() as directory:
            path=Path(directory)/'payload';path.write_bytes(b'correct')
            head=subprocess.CompletedProcess([],1,'','AccessDenied (403)')
            with patch('upload_assets.subprocess.run',return_value=head) as run:
                with self.assertRaises(RuntimeError): upload('key',(path,'audio/ogg'),'https://r2.example','bucket')
                self.assertEqual(run.call_count,1)
