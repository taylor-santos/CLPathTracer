__kernel void add_numbers(__global float4* data, __global float4* data2,
        __local float* local_result, __global float* group_result) {

    float sum;
    float4 input1, input2, sum_vector;
    uint global_addr, local_addr;

    global_addr = get_global_id(0);
    input1 = data[global_addr];
    input2 = data2[global_addr];
    sum_vector = input1 + input2;

    local_addr = get_local_id(0);
    local_result[local_addr] = sum_vector.s0 + sum_vector.s1 +
    sum_vector.s2 + sum_vector.s3;
    barrier(CLK_LOCAL_MEM_FENCE);

    if(get_local_id(0) == 0) {
        sum = 0.0f;
        for(int i=0; i<get_local_size(0); i++) {
            sum += local_result[i];
        }
        group_result[get_group_id(0)] = sum;
    }
}