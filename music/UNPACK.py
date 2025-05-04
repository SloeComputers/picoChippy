#!/usr/bin/env python3

import sys
import os
import zipfile
import gzip
import shutil

def extract_zip(zip_path):
   """ Extract zip file into a suitablenamed sub-dir """

   # Get base name without .zip
   base_name   = os.path.splitext(os.path.basename(zip_path))[0]
   extract_dir = os.path.join(os.path.dirname(zip_path), base_name)

   # Make directory
   os.makedirs(extract_dir, exist_ok=True)

   # Extract
   with zipfile.ZipFile(zip_path, 'r') as zip_ref:
      zip_ref.extractall(extract_dir)

   return extract_dir

def decompress_vgz_files(directory):
   """ Convert .vgz files to .vgm """

   for root, _, files in os.walk(directory):
      for file in files:
         if file.lower().endswith('.vgz'):
            vgz_path = os.path.join(root, file)
            vgm_path = os.path.join(root, os.path.splitext(file)[0] + '.vgm')
                
            with gzip.open(vgz_path, 'rb') as f_in:
               with open(vgm_path, 'wb') as f_out:
                  shutil.copyfileobj(f_in, f_out)
                
            print(f"Decompressed: {vgz_path} → {vgm_path}")

            # Delete original .vgz
            os.remove(vgz_path)
            print(f"Deleted original .vgz: {vgz_path}")

def main():
   if len(sys.argv) != 2:
       print("Usage: UNPACK.py <file.zip>")
       sys.exit(1)

   zip_path = sys.argv[1]

   if not zipfile.is_zipfile(zip_path):
       print(f"ERROR: {zip_path} is not a valid zip file.")
       sys.exit(1)

   extract_dir = extract_zip(zip_path)
   print(f"Extracted to: {extract_dir}")

   decompress_vgz_files(extract_dir)

if __name__ == "__main__":
   main()
