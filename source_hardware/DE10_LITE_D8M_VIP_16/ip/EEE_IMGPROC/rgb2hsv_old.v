module rgb2hsv_old (
    // RGB inputs
    input wire [7:0] red,
    input wire [7:0] green,
    input wire [7:0] blue,
    // HSV outputs
    output reg [8:0] hue,
    output reg [7:0] saturation,
    output reg [7:0] value
);

reg [7:0] max_val, min_val;
reg [7:0] delta;

always @(*) begin

    // Find the maximum and minimum values

    if (red >= green && red >= blue) begin
      max_val = red;
      min_val = (green < blue) ? green : blue;
    end 
    else if (green >= red && green >= blue) begin
      max_val = green;
      min_val = (red < blue) ? red : blue;
    end 
    else begin
      max_val = blue;
      min_val = (red < green) ? red : green;
    end
    
    // Calculate value (V)
    value = max_val;
    delta = max_val - min_val;

    // Calculate saturation (S)
    if (max_val == 0) begin
      saturation = 0;
    end 
    else begin
      saturation = ((delta) * 8'd255)/max_val;
    end

    // Calculate hue (H)
    if (max_val == min_val) begin
      hue = 0;
    end 
    else if (max_val == red) begin
      hue = (green - blue) / (delta);
    end 
    else if (max_val == green) begin
      hue = 2 + (blue - red) / (delta);
    end 
    else if (max_val == blue) begin
      hue = 4 + (red - green) / (delta);
    end

    if (hue < 0) begin
      hue = 60*(hue + 6); //convert hue to 0-360 range
    end
end

endmodule
