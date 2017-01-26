/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) and the Intelligence Advanced Research Projects Activity
 * (IARPA) by employees of the Federal Government in the course of their
 * official duties. Pursuant to title 17 Section 105 of the United States Code,
 * this software is not subject to copyright protection and is in the public
 * domain. NIST and IARPA assume no responsibility whatsoever for its use by
 * other parties, and makes no guarantees, expressed or implied, about its
 * quality, reliability, or any other characteristic.
 */

/*******************************************************************************
 * Decompress and parse information from standarized imagery to confirm that   *
 * Biometric Evaluation framework treats image files as expected.              *
 ******************************************************************************/

#include <iostream>

#include <be_image_image.h>
#include <be_io_utility.h>
#include <be_text.h>

namespace BE = BiometricEvaluation;

/** Print image metadata. */
void
printImageInfo(
    const std::shared_ptr<BE::Image::Image> &image);

/** Write decompressed image to `outputFileName`. */
void
saveRawImage(
    const std::shared_ptr<BE::Image::Image> &image,
    const std::string &outputFileName);

int
main(
    int argc,
    char *argv[])
{
	if (argc <= 1) {
		std::cerr << "Usage: " << argv[0] << " <image> [<image> ...]" <<
		    std::endl;
		return (EXIT_FAILURE);
	}

	int ret{EXIT_SUCCESS};
	for (int i{1}; i < argc; ++i) {
		std::cout << "Reading " << argv[i] << "..." << std::endl;
		std::shared_ptr<BE::Image::Image> image{};
		try {
			image = BE::Image::Image::openImage(argv[i]);
			printImageInfo(image);
			saveRawImage(image, BE::Text::basename(argv[i]) +
			    ".raw");
		} catch (BE::Error::Exception &e) {
			std::cerr << "ERROR: Could not open \"" << argv[i] <<
			    "\" (" << e.whatString() << ")" << std::endl;
			ret = EXIT_FAILURE;
		}
	}

	return (ret);
}

void
printImageInfo(
    const std::shared_ptr<BE::Image::Image> &image)
{
	std::cout << "\tDimensions: " << image->getDimensions() << '\n' <<
	    "\tResolution: " << image->getResolution() << '\n' <<
	    "\tBit Depth: " << image->getBitDepth() << '\n' <<
	    "\tColor Depth: " << image->getColorDepth() << std::endl;
}

void
saveRawImage(
    const std::shared_ptr<BE::Image::Image> &image,
    const std::string &outputFileName)
{
	std::cout << "\tDecompressing... " << std::flush;
	BE::Memory::uint8Array rawData{};
	try {
		rawData = image->getRawData();
		std::cout << "[PASS]" << std::endl;
	} catch (BE::Error::Exception &e) {
		std::cout << "[FAIL]" << std::endl;
		std::cerr << "\tFailed to decompress \"" << outputFileName <<
		    "\" (" << e.whatString() << ")" << std::endl;
		return;
	}

	std::cout << "\tWriting decompressed data as \"" << outputFileName <<
	    "\"... " << std::flush;
	if (BE::IO::Utility::fileExists(outputFileName)) {
		std::cout << "[FAIL] (file exists)" << std::endl;
		return;
	}

	try {
		BE::IO::Utility::writeFile(rawData, outputFileName);
		std::cout << "[PASS]" << std::endl;
	} catch (BE::Error::Exception &e) {
		std::cout << "[FAIL]" << std::endl;
		std::cerr << "\tFailed to write \"" << outputFileName <<
		    "\" (" << e.whatString() << ")" << std::endl;
	}
}

