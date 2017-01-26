<p align="center"><img src="http://nigos.nist.gov:8080/nist/n2n_logo_white.svg.gz" title="N2N Challenge" alt="N2N Challenge Logo" width=30%"></p>

Nail to Nail Fingerprint Capture Challenge
==========================================

Overview
--------
This repository contains the Application Program Interface (API) for
participant-specific one-to-many template generation and identification
algorithms for Intelligence Advanced Research Projects Activity (IARPA)'s
[2017 Nail to Nail Fingerprint Capture Challenge]
(https://www.iarpa.gov/challenges/n2n/n2n.html).
This API is based off the API used for
[Fingerprint Vendor Technology Evaluation (FpVTE) 2012]
(https://www.nist.gov/itl/iad/image-group/fpvte-2012),
by the [National Institute of Standards and Technology (NIST)]
(https://www.nist.gov/itl/iad/image-group), released in the public domain.

Documentation
-------------
An overview of the software evaluation to be run against this API is available
in the [test plan]
(https://github.com/usnistgov/IARPA-N2N/blob/master/doc/testplan/testplan.pdf).

Documentation of the classes and methods that comprise the API are available
within the [API header file]
(https://github.com/usnistgov/IARPA-N2N/blob/master/src/include/n2n.h),
[on the web](https://pages.nist.gov/IARPA-N2N), and in a [stand-alone document]
(https://github.com/usnistgov/IARPA-N2N/blob/master/doc/testplan/testplan.pdf).

Implementing the API
--------------------
A pure-virtual (abstract) class called `N2N::Interface` has been created.
Participants must implement all methods of `N2N::Interface` in a subclass, and
submit this implementation as a shared library. The name of the library must
follow the instructions in `N2N::Interface::getIDs()`. A testing application
will link against the submitted library, instantiate an instance of the
implementation by calling `N2N::Interface::getImplementation()`, and perform
various template generation and identification operations.

Implementing this API requires the use of
[`libbiomeval`](https://github.com/usnistgov/BiometricEvaluation).

Submitting
----------
*Submissions are not yet being accepted.*

Contact
-------
Additional information about the project in general can be obtained by emailing
N2NChallenge@iarpa.gov. Additional information about the API or associated
software evaluation can be obtained by emailing N2NChallenge@nist.gov.

License
-------
This API is released in the public domain. See the
[LICENSE](https://github.com/usnistgov/IARPA-N2N/blob/master/LICENSE.md) and
[DISCLAIMER](https://github.com/usnistgov/IARPA-N2N/blob/master/DISCLAIMER.md)
for details.
