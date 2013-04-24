#include <thmsg.hpp>
#include <fstream>
// Main function
int main(int argc, char* argv[])
{
	if(argc < 3) {
		std::cout << "Usage: -d/-p infile outfile" << std::endl;
		std::cout << " -d: dump a .msg file into a human-readable .txt file" << std::endl;
		std::cout << " -p: repack a .txt file into a .msg file" << std::endl;
		std::cout << std::endl;
		return 0;
	}

	// Dumping mode
	if(strcmp(argv[1],"-d") == 0)
	{
		std::ifstream fin(argv[2], std::ios_base::binary);
		if(!fin){ std::cout << "Failed to open " << argv[2] << std::endl; return 1; }

		// Read 4 bytes in from the file to figure out how many indexes there are.
		//
		// The actual size of the indexspace is times 2, but we'll handle things here
		//  one word at a time, as opposed to one double-word at a time.
		//
		// Each index is 4 bytes/1 word large.  The data stored in each index is
		//  either a pointer, 0x100, or nullspace - in little-endian format.
		uint32_t indexcnt = 0;
		fin.read( (char*)(&indexcnt), 4);
		indexcnt *= 2;

		// Next, we'll read every index and store them in an array
		//  for use in later reconstruction of the MSG file.
		std::vector<uint32_t> indexes;
		for(uint32_t i=0; i<indexcnt; i++)
		{
			uint32_t tmp;
			fin.read( (char*)(&tmp), 4);
			indexes.push_back(tmp);
		}

		// Now that we've reached the end of the indexes (or indices)
		//  we'll begin reading and decrypting the actual messages.
		// Each (TH09: PoFV) message contains a 4 byte header,
		//  2 bytes - unknown use, random header info?
		//  1 byte  - message type
		//  1 byte  - message length
		std::vector<msg> msgdata;

		while( !fin.eof() ) {
			msg nu; nu.indexref=0; nu.data.clear();

			// Keep track of where our messages are, and what index references them, if any.
			nu.pos = (uint32_t)(fin.tellg());
			for(uint16_t i=0; i<indexes.size(); i++) 
			{
				if( indexes.at(i) != 0 &&
					indexes.at(i) != 0x100 &&
					nu.pos == indexes.at(i) ) { nu.indexref = i; }
			}

			fin.read( (char*)(&nu.header), 2);
			fin.read( (char*)(&nu.type), 1);
			fin.read( (char*)(&nu.len), 1);

			if(nu.len>0) {
				nu.data.resize(nu.len);
				fin.read( (char*)(&nu.data[0]) , nu.len);
			}

			msgdata.push_back(nu);
		}
		// Remove the last message, since it's an extra one put in by our while loop.
		msgdata.pop_back();
		fin.close();

		// Now that we have all the data, it's time to process it.
		for(uint32_t i=0; i<msgdata.size(); i++) 
		{
			if( msgdata.at(i).type == MSGTYPE_DIALOGUE ) 
			{
				xorz(&msgdata.at(i).data, 0x77, 7, 16);

				// Go through and remove any nullspace, for ease of reading.
				cleanbytes(&msgdata.at(i).data);
			}
		}

		// Dump all of it to a text file.
		std::ofstream fout(argv[3]);

		//fout << std::hex;
		for(uint32_t i=0; i<indexes.size(); i++) 
		{
			fout << "ptr " << int2hex(indexes.at(i)) << std::endl;
		}
		for(uint32_t i=0; i<msgdata.size(); i++) 
		{
			fout << "msg "
				<< int2hex(msgdata.at(i).header) << " "
				<< int2hex(msgdata.at(i).type) << "  "
				<< int2hex(msgdata.at(i).indexref) << " ";
				//<< int2hex(msgdata.at(i).len) << ":";

			if(msgdata.at(i).type == MSGTYPE_DIALOGUE) 
				fout << byte2str(msgdata.at(i).data);
			 else 
				fout << byte2hex(msgdata.at(i).data);
			

			fout << std::endl;
		}
		fout.close();

		// Make a report. (ayayayaya)
		std::cout << std::endl;
		std::cout << "Index count: " << indexcnt << std::endl;
		std::cout << "MSG count: " << msgdata.size() << std::endl;
	}else if(strcmp(argv[1],"-p") == 0) 
		{
		std::ifstream fin(argv[2]);
		if(!fin){ std::cout << "Failed to open " << argv[2] << std::endl; return 1; }

		// Retrieve all the data from the text file we generated, and store it.
		std::vector<uint32_t> indexes;
		std::vector<msg> msgdata;

		while( !fin.eof() ) {
			std::string tmp;

			fin >> tmp;
			if(tmp.compare("ptr") == 0) {
				// Read pointer location.
				fin >> tmp;
				indexes.push_back( hex2long(tmp) );
			} else if(tmp.compare("msg") == 0) {
				msg nu; nu.len=0; nu.pos=0; nu.indexref=0; nu.data.clear();

				fin >> tmp; nu.header = hex2short(tmp);		// Read 2-byte header.
				fin >> tmp; nu.type = hex2byte(tmp);		// Read 1-byte type.
				fin >> tmp; nu.indexref = hex2short(tmp);	// Read 2-byte index reference.

				// Skip the single space.
				//fin.seekg(1, ios_base::cur);

				getline(fin, tmp);
				if(nu.type == MSGTYPE_DIALOGUE) {
					tmp.erase(0, 1);  // erase the space delimiter
					tmp.push_back(0); // drop a null byte on the end
					while( (tmp.length() % 4) != 0 ) { tmp.push_back(0); } // fill it out until it's fittable into a WORD space

					nu.data = str2bytes(tmp);
					xorz(&nu.data, 0x77, 7, 16);
					nu.len = nu.data.size();
				} else if(tmp.length() > 0) {
					nu.data = hex2bytes(tmp);
					nu.len = nu.data.size();
				}

				msgdata.push_back(nu);
			}
		}
		fin.close();

		uint32_t index_size = (indexes.size() / 2);
		//uint32_t index_space = 4 + ( indexes.size() * 4 );

		std::ofstream fout(argv[3], std::ios_base::binary);

		// Write the header.
		fout.write( (char*)(&index_size), 4);

		// Write the old index table.
		for(uint32_t i=0; i<indexes.size(); i++) 
			fout.write( (char*)(&indexes.at(i)), 4);
		

		// Write the messages.
		for(uint32_t i=0; i<msgdata.size(); i++) {
			msgdata.at(i).pos = fout.tellp();
			fout.write( (char*)(&msgdata.at(i).header), 2) ;
			fout.write( (char*)(&msgdata.at(i).type), 1) ;
			fout.write( (char*)(&msgdata.at(i).len), 1) ;

			if(msgdata.at(i).len>0) {
				for(uint32_t j=0; j<msgdata.at(i).data.size(); j++) {
					fout.write( (char*)(&msgdata.at(i).data.at(j)), 1) ;
				}
			}
		}

		// Traverse the message data to rewrite the old index table
		//  with the new pointers.
		for(uint32_t i=0; i<msgdata.size(); i++) {
			if(msgdata.at(i).indexref>0) {
				fout.seekp(4 + (msgdata.at(i).indexref * 4) );
				fout.write( (char*)(&msgdata.at(i).pos), 4) ;
			}
		}

		// Close the file and finish up.
		fout.close();

	} else { std::cout << "unrecognized switch mode." << std::endl; }

    return 0;
}
