#include "entry.h"
#include "pak.h"

#include "zlib.h"
#include <openssl/sha.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <assert.h>

// Parses PAK; decompress as needed; builds patched pak
PAK::PAK(std::wstring filename) {
	std::string full_filename;
	
	// Load file
	file.open(filename, std::ios::in | std::ios::binary | std::ios::ate);
	if (!file.is_open()) {
		return;
	}
	
	// Get size
	size = file.tellg();
	
	// Get pointers and SHA
	file.seekg(size - (std::streampos)44, std::ios::beg);
	file.read((char*)&magic, sizeof(magic));
	file.read((char*)&fileStart, sizeof(fileStart));
	file.read((char*)&fileSize, sizeof(fileSize));
	file.read(fileSHA1, 20);
	
	// Get base directory
	file.seekg(fileStart, std::ios::beg);
	file.read((char*)&base_size, sizeof(base_size));
	base_dir = new char[base_size];
	file.read(base_dir, base_size);
	
	// Number of files
	file.read((char*)&num_files, sizeof(num_files));
	
	int32_t s;
	uint64_t p1, p2;
	for (int i = 0; i < num_files; i++) {
		
		// Get filename
		file.read((char*)&s, sizeof(s));
		char* f;
		if (s > 0) {
			f = new char[s];
			file.read(f, s);
		}
		else {
			// NB: utf16-encoded filenames will not be paked properly! Unnecessary for now!
			f = new char[-2 * s];
			file.read(f, -2 * s);
		}
		
		// Read data for the file
		ENTRY e;
		file.read((char*)&e.base, sizeof(e.base));
		file.read((char*)&e.size, sizeof(e.size));
		file.read((char*)&e.decomp_size, sizeof(e.decomp_size));
		file.read((char*)&e.is_comp, sizeof(e.is_comp));
		file.read((char*)&e.sha1, sizeof(e.sha1));
		file.read((char*)&e.num_zips, sizeof(unsigned int));
		
		if (e.is_comp) {
			// Pointers to start and end of each zipped segment
			for (int i = 0; i < e.num_zips; i++) {
				file.read((char*)&p1, sizeof(p1)); // Start
				file.read((char*)&p2, sizeof(p2)); // End
				e.chunk_sizes.push_back(p2 - p1);
			}
			file.seekg(1, std::ios::cur);
			file.read((char*)&e.zip_size, sizeof(e.zip_size));
		}
		else {
			e.chunk_sizes.push_back(e.size);
			e.zip_size = e.size;
			file.seekg(1, std::ios::cur);
		}
		
		full_filename = base_dir;
		full_filename += f;
		// Store in map
		m[full_filename] = e;
		
		// Setup for building
		is_modded[full_filename] = false;
		
	}
	
};

PAK::~PAK() {
  if (file.is_open()) {
    file.close();
  }
};

int PAK::LoadFiles(std::wstring filename) {
	std::ifstream file;
	file.open(filename, std::ios::in);
	if (file.is_open()) {
		std::string line;
		while (getline(file, line)) {
			this->Decompress(line);
		}
	}
	return 0;
}

// function to get (decompressed) file given (partial) filename
int PAK::Decompress(std::string filename) {
	ENTRY* e = &m[filename];
	decomp_files[filename] = new unsigned char[e->decomp_size];
	
	// Load directly to output if not compressed
	if (!e->is_comp) {
		file.seekg(e->base + 0x35, std::ios::beg);
		file.read((char*)decomp_files[filename], e->size);
		return 0;
	}
	
	// Read all compressed data
	unsigned char* in = new unsigned char[e->size];
	uint64_t addr = e->base + 0x34 + 0x10 * e->num_zips + 5;
	file.seekg(addr, std::ios::beg);
	file.read((char*)in, e->size);
	
	// Initialize the stream
	z_stream strm;
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = 0;
	strm.next_in = Z_NULL;
	inflateInit(&strm);
	
	// Decompress
	strm.next_in = in;
	strm.avail_in = e->size;
	strm.next_out = decomp_files[filename];
	strm.avail_out = e->decomp_size;
	for (int i = 0; i < e->num_zips; i++) {
		inflate(&strm, Z_NO_FLUSH);
		inflateReset(&strm);
	}
	(void)inflateEnd(&strm);
	delete[] in;

	return 0;
}

int PAK::Compress(std::string filename) {
	ENTRY* e = &m[filename];
	std::vector<unsigned char*> comp;
	
	if (!e->is_comp) {
		// Update SHA
		SHA1(decomp_files[filename], e->size, e->sha1);
		// Store as compressed data
		comp_files[filename] = new unsigned char[e->decomp_size];
		std::memcpy(comp_files[filename], decomp_files[filename], e->decomp_size);
		return 0;
	}
	
	// Initialize the stream
	z_stream strm;
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	deflateInit(&strm, Z_DEFAULT_COMPRESSION);
	
	// Compress
	strm.next_in = decomp_files[filename];
	uint64_t size = 0;
	e->size = 0; // must recalculate
	e->chunk_sizes.clear();
	e->num_zips = 0;
	while (size < e->decomp_size) {
		// Setup next chunk of compressed output
		unsigned char* out = new unsigned char[CHUNK];
		comp.push_back(out);
		strm.next_out = out;
		strm.avail_out = CHUNK;
		// Setup decompressed input
		strm.avail_in = std::min(CHUNK, e->decomp_size - size);
		// Deflate
		deflate(&strm, Z_FINISH);
		// Update entry data
		e->size += CHUNK - strm.avail_out;
		e->chunk_sizes.push_back(CHUNK - strm.avail_out);
		e->num_zips += 1;
		// Prep for next iteration
		deflateReset(&strm);
		size += CHUNK;
	}
	deflateEnd(&strm);
	e->zip_size = e->num_zips == 1 ? e->decomp_size : CHUNK; // either CHUNK for mult zips or TOTAL SIZE < CHUNK for 1 zip
	
    // Store compressed data
	comp_files[filename] = new unsigned char[e->size];
	uint64_t addr = 0;
	for (int i = 0; i < e->num_zips; i++) {
		std::memcpy(&comp_files[filename][addr], comp[i], e->chunk_sizes[i]);
		addr += e->chunk_sizes[i];
	}
	
	// Update SHA
	SHA1(comp_files[filename], e->size, e->sha1);
	
	for (int i = 0; i < e->num_zips; i++) {
		delete[] comp[i];
	}
	
	return 0;
}

void PAK::BuildPak(std::wstring outfile) {
	uint64_t zero = 0;
	ENTRY e;
	uint64_t address;
	std::vector<uint64_t> pointers;
	
	// Don't bother packing if no files are modded!
	int count = 0;
	for (auto const& [filename, val] : is_modded) {
		count += val;
	}
	if (!count) {
		return;
	}
	
	// Compress modded data only
	for (auto const& [filename, val] : is_modded) {
		if (val) {
			// Compress data
			this->Compress(filename);
		}
	}
	
	// output file
	std::fstream output;
	output.open(outfile, std::fstream::in | std::fstream::out | std::ios::binary | std::fstream::trunc);
	
	for (auto const& [filename, value] : comp_files) {
		
		// Store for file section
		pointers.push_back(output.tellp());
		
		// Add entry (include data!)
		e = m[filename];
		output.write((char*)&zero, sizeof(uint64_t));
		output.write((char*)&e.size, sizeof(uint64_t));
		output.write((char*)&e.decomp_size, sizeof(uint64_t));
		output.write((char*)&e.is_comp, sizeof(uint32_t));
		output.write((char*)&e.sha1, sizeof(e.sha1));
		if (e.is_comp) {
			output.write((char*)&e.num_zips, sizeof(e.num_zips));
			address = (uint64_t)output.tellp() + 0x10 * (uint64_t)e.num_zips + 5L;
			std::cout << address << std::endl;
			for (int i = 0; i < e.num_zips; i++) {
				output.write((char*)&address, sizeof(address));
				address += e.chunk_sizes[i];
				output.write((char*)&address, sizeof(address));
				std::cout << "CHUNK SIZE " << e.chunk_sizes[i] << std::endl;
			}
			output.write((char*)&zero, 1);
			output.write((char*)&e.zip_size, sizeof(e.zip_size));
			output.write((char*)comp_files[filename], e.size);
		}
		else {
			output.write((char*)&zero, 5);
			output.write((char*)decomp_files[filename], e.decomp_size);
		}
	}
	
	// Pointer to start of the file section
	uint64_t file_start_pointer = (uint64_t)output.tellp();
	
	// Update the base directory
	this->UpdateBaseDir();
	
	// Output folder base
	output.write((char*)&base_size, sizeof(base_size));
	output.write((char*)base_dir, base_size);
	
	uint32_t num_files = pointers.size();
	output.write((char*)&num_files, sizeof(num_files));
	
	uint32_t filename_size;
	int index = 0; // change to map filename -> pointer?
	for (auto const& [filename, value] : comp_files) {
		std::cout << is_modded[filename] << filename << std::endl;
		if (!is_modded[filename])
			continue;
		
		// Output folder name -- UTF16 files not included!
		filename_size = strlen(&filename[base_size - 1]) + 1;
		output.write((char*)&filename_size, sizeof(filename_size));
		output.write((char*)&filename[base_size - 1], filename_size);
		
		// Output data (pointers, sizes, sha, etc.)
		e = m[filename];
		output.write((char*)&pointers[index], sizeof(uint64_t));
		output.write((char*)&e.size, sizeof(uint64_t));
		output.write((char*)&e.decomp_size, sizeof(uint64_t));
		output.write((char*)&e.is_comp, sizeof(uint32_t));
		output.write((char*)&e.sha1, sizeof(e.sha1));
		if (e.is_comp) {
			output.write((char*)&e.num_zips, sizeof(e.num_zips));
			address = pointers[index] + 0x34L + 0x10 * (uint64_t)e.num_zips + 5L;
			for (int i = 0; i < e.num_zips; i++) {
				output.write((char*)&address, sizeof(address));
				address += e.chunk_sizes[i];
				output.write((char*)&address, sizeof(address));
			}
			output.write((char*)&zero, 1);
			output.write((char*)&e.zip_size, sizeof(e.zip_size));
		}
		else {
			output.write((char*)&zero, 5);
		}
		
		index += 1;
	}
	
	// Size of the file section
	uint64_t file_section_size = (uint64_t)output.tellg() - file_start_pointer;
	
	// Output end of file (magic, filestart, filesize, sha of file section)
	output.write((char*)&zero, 1);
	output.write((char*)&magic, sizeof(magic));
	output.write((char*)&file_start_pointer, sizeof(file_start_pointer));
	output.write((char*)&file_section_size, sizeof(file_section_size));
	
	output.close();
	
	// Load data needed for calculating SHA
	unsigned char* sha_data = new unsigned char[file_section_size];
	output.open(outfile, std::fstream::in | std::ios::binary);
	output.seekg(file_start_pointer, std::ios::beg);
	output.read((char*)sha_data, file_section_size);
	output.close();
	
	// Calculate SHA
	unsigned char file_sha[20];
	SHA1(sha_data, file_section_size, file_sha);
	delete[] sha_data;
	
	// Append SHA to the file
	output.open(outfile, std::ios_base::app | std::ios::binary);
	output.write((char*)file_sha, 20);
	output.close();
}

void PAK::EditFile(std::string filename, uint64_t address, int value) {
	decomp_files[filename][address] = value;
	is_modded[filename] = true;
}

// Longest prefix among modded files only
void PAK::UpdateBaseDir() {
	bool first_file_picked = false;
	std::string first_file;
	uint64_t k, j;
	
	for (auto const& [filename, value] : comp_files) {
		std::cout << is_modded[filename] << filename << std::endl;
		if (!is_modded[filename])
			continue;
		
		if (!first_file_picked) {
			first_file = filename;
			k = first_file.length() - 1;
			while (first_file[k] != 0x2f) {  // just directory name; no filename
				k -= 1;
			}
			first_file_picked = true;
			continue;
		}
		
		k = std::min(k, filename.length());
		j = 0;
		for (uint64_t i = 0; i < k; i++) {
			if (first_file[i] == 0x2f) { // "/"
				j = i;
			}
			if (first_file[i] != filename[i]) {
				k = j;
				break;
			}
		}
	}
	
	if (!first_file_picked)
		return;
	
	std::string t = first_file.substr(0, k + 1); // +1 to include the "/" at the end
	base_dir = new char[k + 2];  // +1 to include "0" at the end
	std::memcpy(base_dir, t.data(), k + 1);
	base_dir[k + 1] = 0;
	base_size = k + 2; // used for cropping base_dir from filenames
}
