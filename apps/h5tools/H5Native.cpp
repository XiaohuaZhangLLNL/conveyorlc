//
// Created by Zhang, Xiaohua on 8/24/21.
//

#include "H5Native.h"
#include <iostream>
#include <string>
#include <H5Cpp.h>
using namespace H5;

using std::cout;
using std::endl;

const H5std_string FILE_NAME( "sp_A0A286YF01_SCGR7_.FR_2_89.04_xxsBL62.7wga_B-em_scores.hdf5" );
const H5std_string DATASET_NAME( "data_ids" );

int main()
{
    try {
        /*
         * Turn off the auto-printing when failure occurs so that we can
         * handle the errors appropriately
         */
        Exception::dontPrint();
        /*
         * Open the specified file and the specified dataset in the file.
         */
        H5File file(FILE_NAME, H5F_ACC_RDONLY);
        DataSet dataset = file.openDataSet(DATASET_NAME);
        /*
        * Get the class of the datatype that is used by the dataset.
        */
        H5T_class_t type_class = dataset.getTypeClass();
        /*
         * Get class of datatype and print message if it's an integer.
         */
        if( type_class == H5T_INTEGER )
        {
            cout << "Data set has INTEGER type" << endl;
            /*
         * Get the integer datatype
             */
            IntType intype = dataset.getIntType();
            /*
             * Get order of datatype and print message if it's a little endian.
             */
            H5std_string order_string;
            H5T_order_t order = intype.getOrder( order_string );
            cout << order_string << endl;
            /*
             * Get size of the data element stored in file and print it.
             */
            size_t size = intype.getSize();
            cout << "Data size is " << size << endl;
        }
        /*
         * Get dataspace of the dataset.
         */
        DataSpace dataspace = dataset.getSpace();
        /*
         * Get the number of dimensions in the dataspace.
         */
        int rank = dataspace.getSimpleExtentNdims();
        /*
         * Get the dimension size of each dimension in the dataspace and
         * display them.
         */
        hsize_t dims_out[2];
        int ndims = dataspace.getSimpleExtentDims( dims_out, NULL);
        cout << "rank " << rank << ", dimensions " <<
             (unsigned long)(dims_out[0]) << " x " <<
             (unsigned long)(dims_out[1]) << endl;
    }
    // catch failure caused by the DataSpace operations
    catch( ... )
    {
        return -1;
    }
    return 0;
}