// -*- lsst-c++ -*-
#include <iostream>

#include "Source.h"
#include "Schema.h"
#include "Measurement.h"
#include "Photometry.h"
#include "Image.h"
#include "Output.h"

#include "AperturePhotometry.h"

/************************************************************************************************************/
//
// Usage: ./main type [type ...]  where type is one of "aper", "psf", and "model"
//
int main(int argc, char **argv) {
    // Create our astrometric measuring object
    MeasureAstrometry *measureAstro = new MeasureAstrometry();
    measureAstro->addAlgorithm("naive");

    // Create our photometric measuring object based on argv
    MeasurePhotometry *measurePhoto = new MeasurePhotometry();
    
    for (int i = 1; i != argc; ++i) {
        measurePhoto->addAlgorithm(argv[i]);
    }
    // Measure the data and retrieve the answers
    Image im = 1.0;

    std::vector<Source::Ptr> sources;

    Source::Ptr s = boost::make_shared<Source>();
    float x = 10, y = 20;
    s->setAstrometry(measureAstro->measure(im, x, y));
    s->setPhotometry(measurePhoto->measure(im, x, y));
    sources.push_back(s);

    Measurement<Photometry> const& v = s->getPhotometry();

    im = 10;
    x = 20, y = 100;
    Source::Ptr s2 = boost::make_shared<Source>();
    s2->setAstrometry(measureAstro->measure(im, x, y));
    s2->setPhotometry(measurePhoto->measure(im, x, y));
    sources.push_back(s2);

    std::cout << *s << std::endl;
    std::cout << *s2 << std::endl;
    std::cout << std::endl;
    //
    // Subclasses of Photometry need a cast to use the accessors that aren't in the base class
    // (This doesn't compile if you don't include a definition of AperturePhotometry)
    //
    // Note that we can use get() to return a value as a double given its name;  the accessor
    // functions can use a templated version of get to return the true type
    //
    Photometry::Ptr photom = *v.begin();
    std::cout << "flux: " << photom->getFlux() << " fluxErr: " << photom->get("fluxErr");
#if defined(APERTURE_PHOTOMETRY_H)
    {
        AperturePhotometry::Ptr aphotom = boost::dynamic_pointer_cast<AperturePhotometry>(photom);

        if (aphotom) {
            std::cout << "  Radius: " << aphotom->getRadius(0);
        }
    }
    std::cout << std::endl << std::endl;
#endif
    //
    // Write out the first Source, using the schema
    //
    showFromSchema(*(*sources.begin()));
    std::cout << std::endl;
    //
    // Write a csv file containing all our measurements, using the schema
    //
    writeCsv(sources, "");
}
