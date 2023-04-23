module test(input D, C, R, RV, RV1, RV2,
            output reg Q);
    always @(posedge C, posedge R)
        if (R)
	        Q <= RV+RV1+RV2;
	    else
	        Q <= D;
endmodule
