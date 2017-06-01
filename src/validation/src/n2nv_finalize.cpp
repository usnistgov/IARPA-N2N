/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <cmath>
#include <iostream>

#include <be_error.h>
#include <be_framework_api.h>
#include <be_io_propertiesfile.h>
#include <be_io_utility.h>
#include <be_time.h>

#include <n2nv_finalize.h>

namespace BE = BiometricEvaluation;
using namespace BE::Framework::Enumeration;

N2N::Validation::Finalize::Arguments
N2N::Validation::Finalize::procargs(
    int argc,
    char *argv[])
{
	static const std::string ConfigDirKey{"Configuration Directory"};
	static const std::string EnrollmentDirKey{"Enrollment Directory"};
	static const std::string EnrollmentRSKey{"Enrollment RecordStore"};
	static const std::string NumNodesKey{"Number of Nodes"};
	static const std::string RAMPerNodeKey{"RAM Per Node"};

	static const std::string NumNodesKeyDefault{"1"};

	static const std::string usage{"Usage: " + std::string(argv[0]) + " "
	    "<properties.conf>\n\nRequired properties:\n"
	    "\t * " + ConfigDirKey + " = /path/to/directory\n"
	    "\t * " + EnrollmentDirKey + " = /path/to/directory\n"
    	    "\t * " + EnrollmentRSKey + " = /path/to/directory\n"
    	    "\t * " + RAMPerNodeKey + " = >0 KiB\n"
   	    "\nOptional properties:\n"
       	    "\t * " + NumNodesKey + " = [1-255] (default: " +
       	        NumNodesKeyDefault + ")\n"
	};

	Finalize::Arguments args;
	if (argc != 2)
		throw BE::Error::StrategyError(usage);

	std::unique_ptr<BE::IO::PropertiesFile> props;
	try {
		props.reset(new BE::IO::PropertiesFile(
		    argv[1], BE::IO::Mode::ReadOnly,
		    {{NumNodesKey, NumNodesKeyDefault}}));
	} catch (const BE::Error::Exception &e) {
		throw BE::Error::StrategyError("Could not open \"" +
		    std::string(argv[1]) + "\" (" + e.whatString() + ")");
	}

	/* Configuration directory */
	try {
		args.configDir = props->getProperty(ConfigDirKey);
	} catch (const BE::Error::ObjectDoesNotExist) {
		throw BE::Error::StrategyError("Missing property: " +
		    ConfigDirKey + '\n' + usage);
	}
	if (!BE::IO::Utility::pathIsDirectory(args.configDir))
		throw BE::Error::StrategyError("Directory for property \"" +
		    ConfigDirKey + "\" (" + args.configDir + ") does not "
		    "exist");

	/* Enrollment directory */
	try {
		args.enrollDir = props->getProperty(EnrollmentDirKey);
	} catch (const BE::Error::ObjectDoesNotExist) {
		throw BE::Error::StrategyError("Missing property: " +
		    EnrollmentDirKey + '\n' + usage);
	}
	if (BE::IO::Utility::fileExists(args.enrollDir) ||
	    BE::IO::Utility::pathIsDirectory(args.enrollDir)) {
		throw BE::Error::StrategyError("Directory for property \"" +
		    EnrollmentDirKey + "\" (" + args.enrollDir + ") already "
		    "exists");
	}

	/* Enrollment template RecordStore */
	try {
		args.enrollRSPath = props->getProperty(EnrollmentRSKey);
	} catch (const BE::Error::ObjectDoesNotExist) {
		throw BE::Error::StrategyError("Missing property: " +
		    EnrollmentRSKey + '\n' + usage);
	}
	try {
		volatile auto const rs = BE::IO::RecordStore::openRecordStore(
		    args.enrollRSPath);
	} catch (const BE::Error::Exception &e) {
		throw BE::Error::StrategyError("Could not open RecordStore for "
		    "property \"" + EnrollmentRSKey + "\" (" +
		    args.enrollRSPath + "): " + e.whatString());
	}

	/* Number of nodes */
	try {
		args.numberOfNodes = props->getPropertyAsInteger(NumNodesKey);
	} catch (const BE::Error::ObjectDoesNotExist) {
		throw BE::Error::StrategyError("Missing property: " +
		    NumNodesKey + '\n' + usage);
	}
	if (args.numberOfNodes == 0)
		throw BE::Error::StrategyError("Invalid value for property \"" +
		    NumNodesKey + "\" (" + std::to_string(args.numberOfNodes) +
		    ')');

	/* RAM per node */
	try {
		args.RAMPerNode = props->getPropertyAsInteger(RAMPerNodeKey);
	} catch (const BE::Error::ObjectDoesNotExist) {
		throw BE::Error::StrategyError("Missing property: " +
		    RAMPerNodeKey + '\n' + usage);
	}
	if (args.RAMPerNode == 0)
		throw BE::Error::StrategyError("Invalid value for property \"" +
		    RAMPerNodeKey + "\" (" + std::to_string(args.RAMPerNode) +
		    ')');

	return (args);
}

int
N2N::Validation::Finalize::run(
    const N2N::Validation::Finalize::Arguments &args)
{
	/* Make enrollment directory */
	if (BE::IO::Utility::makePath(args.enrollDir, S_IRWXU | S_IRWXG) != 0)
		throw BE::Error::StrategyError("Could not create enrollment "
		    "directory (" + args.enrollDir + "): " +
		    BE::Error::errorStr());

	/* Open RecordStore of enrollment templates */
	std::shared_ptr<BE::IO::RecordStore> rs;
	try {
		rs = BE::IO::RecordStore::openRecordStore(args.enrollRSPath);
	} catch (BE::Error::Exception &e) {
		throw BE::Error::StrategyError("Failed to open RecordStore (" +
		    args.enrollRSPath + "): " + e.whatString());
	}

	/* Be gracious with time during validation */
	constexpr uint64_t NintyMinutesAsMicroseconds{120u * 60u *
	    BE::Time::MicrosecondsPerSecond};
	BE::Framework::API<N2N::ReturnStatus> api{};
	api.getWatchdog()->setInterval(std::ceil(rs->getCount() / 1000000.0) *
	    NintyMinutesAsMicroseconds);

	BE::Framework::API<N2N::ReturnStatus>::Result result{};
	const auto lib = N2N::Interface::getImplementation();
	std::cout << "NumNodes RAMPerNode Time State StatusCode Info\n";

	/* Call finalizeEnrollment() until enough resources provided */
	uint8_t adjustedNumNodes{args.numberOfNodes};
	for (;;) {
		result = api.call([&]() -> N2N::ReturnStatus {
			return (lib->finalizeEnrollment(args.configDir,
			    args.enrollDir, adjustedNumNodes, args.RAMPerNode,
			    *rs));
		});

		std::cout << std::to_string(adjustedNumNodes) << " " <<
		    std::to_string(args.RAMPerNode) << " " <<
		    result.elapsed << " " <<
		    std::to_string(to_int_type(result.currentState)) << " ";

		if (result) {
			std::cout << std::to_string(static_cast<
			    std::underlying_type<N2N::StatusCode>::type>(
			    result.status.code)) << " [<[" <<
			    result.status.info << "]>]" << std::endl;

			if (result.status.code ==
			    StatusCode::InsufficientResources) {
				if (adjustedNumNodes >= 5) {
					throw BE::Error::StrategyError("Could "
					    "not complete finalizeEnrollment() "
					    "with >= 5 nodes");
				} else {
					++adjustedNumNodes;
					continue;
				}
			} else {
				break;
			}
		} else {
			std::cout << "NA [<[]>]" << std::endl;
			throw BE::Error::StrategyError("Exceptional condition "
			    "encountered during finalizeEnrollment()");
		}
	}

	return (static_cast<std::underlying_type<N2N::StatusCode>::type>(
	    result.status.code));
}

int
main(
    int argc,
    char *argv[])
try {
	using namespace N2N::Validation;
	return (Finalize::run(Finalize::procargs(argc, argv)));
} catch (const BE::Error::Exception &e) {
	std::cerr << e.what() << std::endl;
	return (EXIT_FAILURE);
}
