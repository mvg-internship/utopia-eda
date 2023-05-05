module norGate(cOut, x0, x1);
  input x0, x1;
  output cOut;
  wire cOut;

  nor g4 (cOut, x0, x1);
endmodule
