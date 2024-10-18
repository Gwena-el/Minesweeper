
struct dynamic_array_int
{
    int* element;
    size_t length;
    size_t bucket_size;
};


// -----------------------------------------------------------------------------------------
internal dynamic_array_int new_int_array(int bucket_size = 4)
{
    dynamic_array_int return_array;
    
    return_array.element = (int *)malloc(sizeof(int) * bucket_size);
    assert(return_array.element != NULL);
    memset(return_array.element, 0, sizeof(int) * bucket_size);
    
    return_array.length = 0;
    return_array.bucket_size = bucket_size;
    
    return return_array;
}

// -----------------------------------------------------------------------------------------
internal void clear_array(dynamic_array_int *array)
{
    if(array->length > 0)
    {
        for(size_t i = 0; i < array->length; ++i)
            array->element[i] = 0;

        array->length = 0;
    }
}

// -----------------------------------------------------------------------------------------
// @TODO: What do we do if memory allocation fails?? data simply lost?...
internal void push_back(dynamic_array_int *array, int n)
{
    if(array->length >= array->bucket_size)
    {
        array->bucket_size *= 2;
        array->element = (int *)realloc(array->element, sizeof(int) * array->bucket_size);
        assert(array->element != NULL);
    }

    array->element[array->length++] = n;
}

// -----------------------------------------------------------------------------------------
internal void pop(dynamic_array_int *array)
{
    if(array->length > 0)
    {
        for(size_t i = 0; i < array->length - 1; ++i)
            array->element[i] = array->element[i+1];

        --array->length;
    }
}


