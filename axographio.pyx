import numpy as np
cimport numpy as np

cdef extern from "stdlib.h":
    ctypedef int size_t
    void* malloc(size_t num)
    void free(void*)
    void* memcpy(void* destination, void* source, size_t num)
    void* memset(void* destination, int source, size_t num)

cdef extern from "axograph_readwrite/fileUtils.h":
    ctypedef void* AGDataRef
    ctypedef extern char* const_char_ptr "const char*"
    AGDataRef NewFile( const_char_ptr fileName )
    AGDataRef OpenFile( const_char_ptr fileName )
    void CloseFile( AGDataRef dataRefNum )


cdef extern from "axograph_readwrite/AxoGraph_ReadWrite.h":
    ctypedef int int32_t
    enum ag_errors:
        kAG_MemoryErr, kAG_FormatErr, kAG_VersionErr

    enum ColumnType:
        IntType,
        DoubleType,
        BoolType,
        StringType,
        ShortArrayType,
        IntArrayType,
        FloatArrayType,
        DoubleArrayType,
        BoolArrayType,
        SeriesArrayType,
        ScaledShortArrayType,
        StringArrayType,
        ReferenceType

    struct SeriesArray:
        double firstValue
        double increment


    struct ScaledShortArray:
        double scale
        double offset
        short *shortArray

    struct ColumnData:
        ColumnType type
        int32_t points
        int32_t titleLength
        unsigned char *title
        # the rest of this structure is a union:
        short *shortArray
        int32_t *intArray
        float *floatArray
        double *doubleArray
        SeriesArray seriesArray
        ScaledShortArray scaledShortArray

    int kAxoGraph_Graph_Format
    int kAxoGraph_Digitized_Format
    int kAxoGraph_X_Format

    int AG_GetFileFormat( AGDataRef refNum, int *fileFormat )

    int AG_GetNumberOfColumns( AGDataRef refNum, int fileFormat,
            int32_t *numberOfColumns )

    int AG_ReadColumn( AGDataRef refNum, int fileFormat,
            int columnNumber, ColumnData *columnData )

    int AG_ReadFloatColumn( AGDataRef refNum, int fileFormat,
            int columnNumber, ColumnData *columnData )

    int AG_WriteHeader( AGDataRef refNum, int fileFormat, int numColumns )

    int AG_WriteColumn( AGDataRef refNum, int fileFormat,
            int columnNumber, ColumnData *columnData )



# supported file formats
old_graph_format = kAxoGraph_Graph_Format #: pre-Axograph X graph format
old_digitized_format = kAxoGraph_Digitized_Format #: pre-Axograph X format
axograph_x_format = kAxoGraph_X_Format #: Axograph X format

newest_format = axograph_x_format #: the most current format supported
supported_formats = [old_graph_format, old_digitized_format, axograph_x_format]

class file_contents:
    """The contents of an axograph data file

    names is a list of column names.

    data contains the contents of each column.

    fileformat is the file format to use; currently old_graph_format,
        old_digitized_format, and axograph_x_format are supported,
        with newest_format being a synonym for the most recent format
        supported (currently axograph_x_format).

    """
    def __init__(self, names, data, fileformat = newest_format):
        self.names = names
        self.data = data
        self.fileformat = fileformat

    def write(self, filename):
        """Write this file to the given filename"""
        cdef int result
        cdef ColumnData columndata

        # open the file
        cdef AGDataRef file = NewFile(filename)
        if file == NULL:
            raise IOError('file not found')

        try:

            # write file the header
            result = AG_WriteHeader( file, self.fileformat, len(self.data) )
            if result != 0:
                raise IOError((result,
                    'AG_WriteHeader returned error %d' % result))

            # write each column
            for i in range(len(self.data)):
                prepare_columndata(&columndata, i, self.fileformat,
                        self.names[i], self.data[i])

                result = AG_WriteColumn(file, self.fileformat, i, &columndata)
                if result != 0:
                    raise IOError((result,
                        'AG_WriteColumn returned error %d' % result))
                free_columndata(&columndata)

        finally:
            CloseFile(file)



class _getitem_iterator:
    """A simple iterator for objects that support __getitem__ and __len__

    This class provides a simple (but slow) iterator that can be added to
    classes that already support __getitem__ and __len__.

    This can be created for an object

    >>> l = [3,1,4]
    >>> i = _getitem_iterator(l)

    __iter__ will return itself, as required in the language spec

    >>> i is iter(i)
    True

    next can be used to walk through the items in the list, eventually
    raising a StopIteration exception.

    >>> i.next()
    3
    >>> i.next()
    1
    >>> i.next()
    4
    >>> i.next()
    Traceback (most recent call last):
        ...
    StopIteration

    """

    def __init__(self, array):
        self.array = array
        self.index = 0

    def __iter__(self):
        return self

    def next(self):
        if self.index >= len(self.array):
            raise StopIteration()
        self.index += 1
        return self.array[self.index - 1]



class linearsequence:
    """A sequence of terms of the form  a_k = m * k + b,   0 <= k < n

    Since samples are typically taken at a fixed frequency, Axograph
    stores the time column as just a starting time (b), number of points (n),
    and interval between points (m).  This class exposes this data as a
    sequence which can easily be converted to an array using numpy.asarray.

    >>> seq = linearsequence(5, 1.125, 0.25)
    >>> len(seq)
    5

    This supports integer indices:

    >>> seq[0]
    1.125
    >>> seq[1]
    1.375
    >>> seq[4]
    2.125
    >>> seq[-1]
    2.125
    >>> seq[-5]
    1.125

    It also works with slices:

    >>> seq[1:11:2]
    array([1.375, 1.875])


    Bad indices will result in an exceptions

    >>> seq[5]
    Traceback (most recent call last):
        ...
    IndexError: index out of bounds
    >>> seq[-6]
    Traceback (most recent call last):
        ...
    IndexError: index out of bounds
    >>> seq['foo']
    Traceback (most recent call last):
        ...
    TypeError: not a valid index type


    This can be converted to an array using numpy.asarray:

    >>> import numpy
    >>> numpy.asarray(seq)
    array([1.125, 1.375, 1.625, 1.875, 2.125])

    or iterated over:

    >>> sum(iter(seq))
    8.125

    """

    def __init__(self, numpoints, start, step):
        """ Creates a new sequence

        Creates a new linear sequence whose first value is start, and where
        each subsequent value is step larger than the value before it.
        """
        self.numpoints = numpoints
        self.start = start
        self.step = step

    def __getitem__(self, index):
        """Implements the s[n] operator"""
        if isinstance(index, int):
            if index < -self.numpoints or index >= self.numpoints:
                raise IndexError('index out of bounds')
            elif index < 0:
                return (self.numpoints + index) * self.step + self.start
            else:
                return index * self.step + self.start
        elif isinstance(index, slice):
            return (np.arange(*index.indices(self.numpoints)) * self.step
                    + self.start)
        else:
            raise TypeError('not a valid index type')

    def __len__(self):
        """Implements len(s)"""
        return self.numpoints

    def __array__(self, dtype=np.float64):
        """Implements numpy.asarray(s)"""
        return np.linspace(self.start, self.start + self.step * self.numpoints,
                self.numpoints, endpoint=False)

    def __iter__(self):
        """Implements iter(s)"""
        return _getitem_iterator(self)



def aslinearsequence(x):
    """ Convert an object to a linear sequence

    If x is a linear sequence, this function will just return x.
    Otherwise, it will try converting x to an array and checking if the
    points form a linear sequence; if so it will return the sequence,
    otherwise it will raise a TypeError.

    >>> x = [1.2, 2.3, 3.4, 4.5]
    >>> y = aslinearsequence(x)
    >>> x is y
    False
    >>> np.asarray(y)
    array([1.2, 2.3, 3.4, 4.5])
    >>> z = aslinearsequence(y)
    >>> y is z
    True
    >>> aslinearsequence([1.2, 2.3, 4.4, 4.5])
    Traceback (most recent call last):
        ...
    TypeError: Data could not be converted to a linear sequence

    """
    if isinstance(x, linearsequence): # nothing to do
        return x
    else:
        x = np.asarray(x)

        if len(x) == 0:
            return linearsequence(0, 0., 0.)
        elif len(x) == 1:
            return linearsequence(1, x[0], 0)
        else:
            step = (x[-1] - x[0]) / (len(x) - 1)

            diffs = (x[1:] - x[:-1]) / step - 1
            if (np.max(np.abs(diffs)) > 1e-6):
                raise TypeError(
                        'Data could not be converted to a linear sequence')

            return linearsequence(len(x), x[0], step)



class scaledarray:
    """A sequence of samples as 16 bit values with scaling and offsets

    This class represents a data format used by axograph, where each value
    is a single 16 bit int.  To convert these values to the actual
    measurements, one multiplies by an scale factor and adds an offset.

    This supports the standard sequence operations, and can be converted to
    an array using numpy.asarray.

    >>> import numpy
    >>> sa = scaledarray(numpy.array([2,7,1,8]), 0.5, 0.25)
    >>> numpy.asarray(sa)
    array([1.25, 3.75, 0.75, 4.25])
    >>> len(sa)
    4

    >>> sa[1]
    3.75
    >>> sum(iter(sa))
    10.0

    """
    def __init__(self, data, scale, offset):
        """Create a scaledarray

        Creates a new scaledarray where each element is equal to its
        corresponding element in data times a scaling factor plus an offset.

        """
        self.data = np.asarray(data, dtype=np.int16)
        self.scale = scale
        self.offset = offset

    def __getitem__(self, index):
        """Implements the s[n] operator"""
        return self.data[index] * self.scale + self.offset

    def __len__(self):
        """Implements len(s)"""
        return len(self.data)

    def __array__(self, dtype=np.float64):
        """Implements numpy.asarray(s)"""
        return np.asarray(self.data * self.scale + self.offset, dtype=dtype)

    def __iter__(self):
        """Implements iter(s)"""
        return _getitem_iterator(self)



def asscaledarray(x, forceoffset = None):
    """ Convert an object to a scaled array

    If x is a scaled array, this function will just return x.
    Otherwise, it will try converting x to an array of doubles and
    distributing the values over the given range.  If forceoffset is
    given it will be used as the offset; otherwise the mean will be used.

    >>> x = [1.25, 8.125, 6.75, 4.625, 9.25]
    >>> y = asscaledarray(x)
    >>> x is y
    False
    >>> np.asarray(y).round(3)
    array([1.25 , 8.125, 6.75 , 4.625, 9.25 ])
    >>> z = asscaledarray(y)
    >>> y is z
    True
    >>> q = asscaledarray(y, forceoffset = 1.3)
    >>> z is q
    False
    >>> np.asarray(y).round(3)
    array([1.25 , 8.125, 6.75 , 4.625, 9.25 ])
    >>> z.offset
    5.25
    >>> q.offset
    1.3

    """
    if isinstance(x, scaledarray) and (
            forceoffset is None or forceoffset == x.offset):
        # nothing to do
        return x
    else:
        x = np.asarray(x)

        if len(x) == 0:
            if forceoffset is None:
                forceoffset = 0.
            return scaledarray(np.array([], dtype=np.int16), 1., forceoffset)
        elif len(x) == 1:
            if forceoffset is None:
                forceoffset = 0.
            return scaledarray(np.array([1], dtype=np.int16),
                    x[0] - forceoffset, forceoffset)
        else:
            if forceoffset is None:
                forceoffset = (x.max() + x.min())/2
            scale = np.max(np.abs(x - forceoffset)) / 32767

            return scaledarray(
                np.array((x - forceoffset) / scale).astype(np.int16),
                scale, forceoffset)



cdef prepare_columndata(ColumnData* columndata, colnum, fileformat, name, data):
    """Use the data in a python sequence to fill out a C ColumnData struct"""
    cdef np.ndarray[np.int16_t, ndim=1] shortdata
    cdef np.ndarray[np.int32_t, ndim=1] intdata
    cdef np.ndarray[np.float32_t, ndim=1] floatdata
    cdef np.ndarray[np.float64_t, ndim=1] doubledata

    memset(columndata, 0, sizeof(columndata))

    # fill in the column name
    columndata.titleLength = 2*len(name)
    columndata.title = <unsigned char*>malloc(columndata.titleLength+2)
    memcpy(columndata.title, <char*>name, len(name))
    columndata.title[len(name)] = 0

    # fill in the number of data points
    columndata.points = len(data)

    # down-convert the data to floats for the older axograph formats
    # (since they only support floats)
    if fileformat == old_digitized_format:
        if colnum == 0:
            data = aslinearsequence(data)
        else:
            data = asscaledarray(data, forceoffset = 0.)
    elif fileformat == old_graph_format:
        data = np.asarray(data, dtype=np.float32)

    # create a column of the appropriate type
    if isinstance(data, linearsequence):
        columndata.type = SeriesArrayType
        columndata.seriesArray.firstValue = data.start
        columndata.seriesArray.increment = data.step
    elif isinstance(data, scaledarray):
        columndata.type = ScaledShortArrayType
        columndata.scaledShortArray.scale = data.scale
        columndata.scaledShortArray.offset = data.offset
        shortdata = data.data
        columndata.scaledShortArray.shortArray = <short*>malloc(
                columndata.points * sizeof(short))
        memcpy(columndata.scaledShortArray.shortArray,
                <short*>shortdata.data,
                columndata.points * sizeof(short))
    else:
        # convert it to an array for further processing
        array = np.asarray(data)

        if array.dtype == np.int16:
            columndata.type = ShortArrayType
            shortdata = array
            columndata.shortArray = <short*>malloc(
                    columndata.points * sizeof(short))
            memcpy(columndata.shortArray, <short*>shortdata.data,
                    columndata.points * sizeof(short))
        elif array.dtype == np.int32:
            columndata.type = IntArrayType
            intdata = array
            columndata.intArray = <int32_t*>malloc(
                    columndata.points * sizeof(int32_t))
            memcpy(columndata.intArray, <int32_t*>intdata.data,
                    columndata.points * sizeof(int32_t))
        elif array.dtype == np.float32:
            columndata.type = FloatArrayType
            floatdata = array
            columndata.floatArray = <float*>malloc(
                    columndata.points * sizeof(float))
            memcpy(columndata.floatArray, <float*>floatdata.data,
                    columndata.points * sizeof(float))
        elif array.dtype == np.float64:
            columndata.type = DoubleArrayType
            doubledata = array
            columndata.doubleArray = <double*>malloc(
                    columndata.points * sizeof(double))
            memcpy(columndata.doubleArray, <double*>doubledata.data,
                    columndata.points * sizeof(double))
        else:
            raise TypeError("Unsupported column data type %s"
                    % repr(array.dtype))



cdef convert_columndata(ColumnData* columndata):
    """Convert the data in a C ColumnData struct to a python sequence"""
    cdef np.ndarray[np.int16_t, ndim=1] shortdata
    cdef np.ndarray[np.int32_t, ndim=1] intdata
    cdef np.ndarray[np.float32_t, ndim=1] floatdata
    cdef np.ndarray[np.float64_t, ndim=1] doubledata

    if columndata.type == ShortArrayType:
        shortdata = np.empty(columndata.points, dtype = np.int16)
        memcpy(shortdata.data, columndata.shortArray,
                columndata.points * sizeof(short))
        return shortdata
    elif columndata.type == IntArrayType:
        intdata = np.empty(columndata.points, dtype = np.int32)
        memcpy(intdata.data, columndata.intArray,
                columndata.points * sizeof(int32_t))
        return intdata
    elif columndata.type == FloatArrayType:
        floatdata = np.empty(columndata.points, dtype = np.float32)
        memcpy(floatdata.data, columndata.floatArray,
                columndata.points * sizeof(float))
        return floatdata
    elif columndata.type == DoubleArrayType:
        doubledata = np.empty(columndata.points, dtype = np.float64)
        memcpy(doubledata.data, columndata.doubleArray,
                columndata.points * sizeof(double))
        return doubledata
    elif columndata.type == SeriesArrayType:
        return linearsequence(columndata.points,
                columndata.seriesArray.firstValue,
                columndata.seriesArray.increment)
    elif columndata.type == ScaledShortArrayType:
        shortdata = np.empty(columndata.points, dtype = np.int16)
        memcpy(shortdata.data, columndata.scaledShortArray.shortArray,
                columndata.points * sizeof(short))
        return scaledarray(shortdata, columndata.scaledShortArray.scale,
            columndata.scaledShortArray.offset)
    else:
        raise IOError('Unsupported column type %d' % columndata.type)



cdef free_columndata(ColumnData* columndata):
    """Free the memory used by a columndata structure"""

    free(<char*>columndata.title)
    columndata.title = NULL
    columndata.titleLength = 0

    if columndata.type == ShortArrayType:
        free(columndata.shortArray)
        columndata.shortArray = NULL
    elif columndata.type == IntArrayType:
        free(columndata.intArray)
        columndata.intArray = NULL
    elif columndata.type == FloatArrayType:
        free(columndata.floatArray)
        columndata.floatArray = NULL
    elif columndata.type == DoubleArrayType:
        free(columndata.doubleArray)
        columndata.doubleArray = NULL
    elif columndata.type == SeriesArrayType:
        pass
    elif columndata.type == ScaledShortArrayType:
        free(columndata.scaledShortArray.shortArray)
        columndata.scaledShortArray.shortArray = NULL
    else:
        raise IOError('Unsupported column type %d' % columndata.type)



def read(char* filename):
    """Read an Axograph file

    Read an Axograph file from disk and return the contents as an
    axographio.filedata object.

    """
    cdef int fileformat = 0
    cdef int result
    cdef int32_t numcolumns
    cdef ColumnData columndata
    cdef unsigned int i

    # open the file
    cdef AGDataRef file = OpenFile(filename)
    if file == NULL:
        raise IOError('file not found')

    try:
        # figure out the file format
        result = AG_GetFileFormat( file, &fileformat )
        if result != 0:
            if result == kAG_FormatErr:
                raise IOError('file is not in AxoGraph format')
            elif result == kAG_VersionErr:
                raise IOError('file is not in AxoGraph format')
            else:
                raise IOError((result,
                    'AG_GetFileFormat returned error %d' % result))

        # read in the number of columns
        result = AG_GetNumberOfColumns(file, fileformat, &numcolumns)
        if result != 0:
            raise IOError((result,
                'AG_GetNumberOfColumns returned error %d' % result))
        elif numcolumns < 0:
            raise IOError('number of columns was negative')

        # read in each column of data
        colnames = []
        coldata = []
        for colnum in range(numcolumns):
            result = AG_ReadColumn(file, fileformat, colnum, &columndata)
            if result != 0:
                raise IOError((result,
                    'AG_ReadColumn returned error %d' % result))

            if <char*>columndata.title is None:
                colname = '' #'Column %d' % colnum
            else:
                colname = <char*>(columndata.title)
            colnames += [colname]
            coldata += [convert_columndata(&columndata)]
            free_columndata(&columndata)

    finally:
        CloseFile(file)

    return file_contents(colnames, coldata, fileformat)
