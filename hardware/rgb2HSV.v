module rbg2HSV (
    //rgb inputs
    input wire [7:0] red,
    input wire [7:0] green,
    input wire [7:0] blue,
    //HSV outputs
    output reg [8:0] hue,
    output reg [7:0] saturation,
    output reg [7:0] value
);


// RGB to HSV conversion
reg [7:0] max_val, min_val;
reg [7:0] delta;
reg [8:0] hue_prime;

always@(red,green,blue) begin
    // Find the maximum and minimum values
  max_val = red > green ? (red > blue ? red : blue) : (green > blue ? green : blue);
  min_val = red < green ? (red < blue ? red : blue) : (green < blue ? green : blue);
  
  // Calculate value (V)
  value = max_val;
  
  // Calculate saturation (S)
  delta = max_val - min_val;
  saturation = (max_val == 0) ? 0 : delta / max_val;
  //saturation = 40;
  
  //Calculate hue (H)

  if (delta == 0) begin
    hue = 0;
  end

  else if (max_val == red) begin
    hue_prime = 60 * ((green - blue) / delta);
    hue = hue_prime >= 0 ? hue_prime : hue_prime + 360;
  end 
  else if (max_val == green) begin
    hue_prime = 60 * (2 + ((blue - red) / delta));
    hue = hue_prime;
  end 
  else if (max_val == blue) begin
    hue_prime = 60 * (4 + ((red - green) / delta));
    hue = hue_prime;
  end
end

endmodule
