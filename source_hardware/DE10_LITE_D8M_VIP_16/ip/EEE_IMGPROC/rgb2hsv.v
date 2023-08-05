module rgb2hsv (
    // RGB inputs
    input wire [7:0] red,
    input wire [7:0] green,
    input wire [7:0] blue,
    // HSV outputs
    output wire [7:0] hue,
    output wire [7:0] saturation,
    output wire [7:0] value
);

wire [7:0] max_val;
wire [7:0] min_val;
wire [7:0] delta;

//find min/max values
assign max_val = red > green ? (red > blue ? red : blue) : (green > blue ? green : blue);
assign min_val = red < green ? (red < blue ? red : blue) : (green < blue ? green : blue);


//Calculate saturation 
assign delta = max_val - min_val;
assign saturation = (max_val == 0 || delta == 0) ? 8'b0 : ((255*delta) / max_val);

//Calculate value
assign value = max_val;

//Calculate hue

assign hue = (max_val == 0 || delta == 0) ? 8'b0 : (max_val == red) ? (43*(green - blue)/delta) : (max_val == green) ? ((43*(blue - red)/delta) + 85) : ((43*(red - green)/delta) + 171);

endmodule

