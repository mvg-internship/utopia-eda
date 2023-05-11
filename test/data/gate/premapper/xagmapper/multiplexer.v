module multiplexer(cOut, x0, x1, x2);
  input x0, x1, x2;
  output cOut;
  wire n1, n2, n3, cOut;

  not g1 (n1, x2);
  and g2 (n2, x0, n1);
  and g3 (n3, x1, x2);
  or g4 (cOut, n3, n2);
endmodule
