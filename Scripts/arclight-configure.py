#!/usr/bin/python3

import os
from os import path
import platform
import shutil
import io
import urllib.request
import zipfile

arclight_root = path.realpath(path.join(path.dirname(path.realpath(__file__)), ".."))

def download_and_unzip(url: str, extract_path: str):
	print(f"Downloading {url}...")
	f = urllib.request.urlopen(url)
	zf = zipfile.ZipFile(io.BytesIO(f.read()))
	zf_files = zf.namelist()
	for file in zf_files:
		print(f"Extracting {file}...")
		zf.extract(file, path=extract_path)
	

def configure_linux():
	pass

def configure_windows():
	os.chdir(arclight_root)

	# Check for SDL2
	if not path.isdir("thirdparty/SDL2"):
		print("Failed to find SDL2, Please download SDL development libraries and extract to thirdparty/SDL2")
		return 1

	shutil.copy("Scripts/sdl2-config.cmake", "thirdparty/SDL2")

	# Check for ICU
	if not path.exists("thirdparty/icu4c/lib64/icuuc.lib"):
		icu_url = "https://github.com/unicode-org/icu/releases/download/release-69-1/icu4c-69_1-Win64-MSVC2019.zip"
		download_and_unzip(icu_url, "thirdparty/icu")

	# Check for Vulkan SDK
	return 0

def configure():
	os_name = platform.system().lower()
	if os_name == 'windows':
		configure_windows()
	elif os_name == 'linux':
		configure_linux()
	else:
		print(f"Unknown platform: '{os_name}'")
		return

if __name__ == '__main__':
	configure()