module EEE_IMGPROC(
	// global clock & reset
	clk,
	reset_n,
	
	// mm slave
	s_chipselect,
	s_read,
	s_write,
	s_readdata,
	s_writedata,
	s_address,

	// stream sink
	sink_data,
	sink_valid,
	sink_ready,
	sink_sop,
	sink_eop,
	
	// streaming source
	source_data,
	source_valid,
	source_ready,
	source_sop,
	source_eop,
	
	// conduit
	mode
	
);


// global clock & reset
input	clk;
input	reset_n;

// mm slave
input							s_chipselect;
input							s_read;
input							s_write;
output	reg	[31:0]	s_readdata;
input	[31:0]				s_writedata;
input	[2:0]					s_address;


// streaming sink
input	[23:0]            	sink_data;
input								sink_valid;
output							sink_ready;
input								sink_sop;
input								sink_eop;

// streaming source
output	[23:0]			  	   source_data;
output								source_valid;
input									source_ready;
output								source_sop;
output								source_eop;

// conduit export
input                         mode;

////////////////////////////////////////////////////////////////////////
//
parameter IMAGE_W = 11'd640;
parameter IMAGE_H = 11'd480;
parameter MESSAGE_BUF_MAX = 256;
parameter MSG_INTERVAL = 6;
parameter BB_COL_DEFAULT = 24'h00ff00;


wire [7:0]   red, green, blue, grey;
wire [7:0]   red_out, green_out, blue_out;

wire [7:0] saturation, value, hue;

wire         sop, eop, in_valid, out_ready;
////////////////////////////////////////////////////////////////////////

rgb2hsv colourspace_conversion(
	.red(red),
	.green(green),
	.blue(blue),

	.hue(hue),
	.saturation(saturation),
	.value(value)
);

////////////////////////////////////////////////////////////////////////

// Detect red areas
wire red_detect;
assign red_detect = red[7] & ~green[7] & ~blue[7];

reg found_red;
reg found_yellow;
reg found_blue;
reg found_white;

always@(*) begin
	found_blue = 0;
	found_red = 0;
 	found_white = 0;
	found_yellow = 0;
	///using matlab values (didnt work), so tweaking

		if(((hue < 15 && hue > 0) || (hue > 235 && hue < 255)) && (saturation > 178) && (saturation < 255) && (value > 156) && (value < 255))begin
			found_red = 1;
		end

		else if((hue > 130) && (hue < 178) && (saturation > 100) && (saturation < 255) && (value > 106) && (value < 255))begin
			found_blue = 1;
		end

		else if((hue > 12) && (hue < 35) && (saturation > 122) && (saturation < 255) && (value > 139) && (value < 255))begin
			found_yellow = 1;
		end

		if((hue > 0) && (hue < 245) && (saturation > 0) && (saturation < 112) && (value > 40) && (value < 255))begin
			found_white = 1;
		end
end

// Find boundary of cursor box

// Highlight detected areas
//wire [23:0] red_high;
assign grey = green[7:1] + red[7:2] + blue[7:2]; //Grey = green/2 + red/4 + blue/4
//assign red_high  =  red_detect ? {8'd255, 8'd192, 8'd203} : {grey, grey, grey}; //allow the RGB detection of red but make it pink

//Similiarly for the resrt of the colours"
wire [23:0] red_high;
wire [23:0] blue_high;
wire [23:0] yellow_high;
wire [23:0] white_high;

assign red_high   = found_red ? {8'hff, 8'd0, 8'd0} : blue_high; //{grey, grey, grey}; // Red areas highlighted as red - was red detect
assign blue_high  = found_blue ? {8'd0, 8'd0, 8'd255} : yellow_high; // Blue areas highlighted as blue
assign yellow_high = found_yellow ? {8'd255, 8'd255, 8'd0} : white_high; // Yellow areas highlighted as yellow
assign white_high = found_white ? {8'd255, 8'd255, 8'd255} : {grey, grey, grey}; // White areas highlighted as white

// Show bounding box
reg [23:0] new_image;
reg [10:0] left_red, right_red, top_red, bottom_red;
reg [10:0] left_blue, right_blue, top_blue, bottom_blue;
reg [10:0] left_yellow, right_yellow, top_yellow, bottom_yellow;
reg [10:0] left_white, right_white, top_white, bottom_white;

wire bb_active_red;
assign bb_active_red = (((x < right_red) & (x > left_red)) & ((y == bottom_red) | (y == top_red))) | (((y < bottom_red) & (y > top_red)) & ((x == left_red) | (x == right_red)));
//new logic

wire bb_active_blue;
assign bb_active_blue = (((x < right_blue) & (x > left_blue)) & ((y == bottom_blue) | (y == top_blue))) | (((y<bottom_blue) & (y>top_blue)) & ((x == left_blue) | (x == right_blue)));

wire bb_active_yellow;
assign bb_active_yellow = (((x<right_yellow) & (x>left_yellow)) & ((y == bottom_yellow) | (y == top_yellow))) | (((y<bottom_yellow) & (y>top_yellow)) & ((x == left_yellow) | (x == right_yellow)));

wire bb_active_white;
assign bb_active_white  = (((x<right_white) & (x>left_white)) & ((y == bottom_white) | (y == top_white))) | (((y<bottom_white) & (y>top_white)) & ((x == left_white) | (x == right_white)));

//assign new_image = bb_active_red ? red_high : (bb_active_blue ? blue_high : (bb_active_yellow ? yellow_high : (bb_active_white ? white_high : {grey, grey, grey})));
always @(*) begin
	// if (bb_active_red || bb_active_blue || bb_active_yellow || bb_active_white) begin 
	// 	new_image <= {8'd255, 8'd255, 8'd255};
	// end
	if(bb_active_red)begin
		new_image <= {8'd255, 8'd0, 8'd0};
	end
	else if(bb_active_blue)begin
		new_image <= {8'd0, 8'd0, 8'd255};
	end
	else if(bb_active_yellow)begin
		new_image <= {8'd255, 8'd255, 8'd0};
	end
	else if(bb_active_white)begin
		new_image <= {8'd255, 8'd255, 8'd255};
	end
	else begin
		new_image <= red_high;
	end
end

// Switch output pixels depending on mode switch
// Don't modify the start-of-packet word - it's a packet discriptor
// Don't modify data in non-video packets
assign {red_out, green_out, blue_out} = (mode & ~sop & packet_video) ? new_image : {red,green,blue};
//assign {red_out, green_out, blue_out} = (mode & ~sop & packet_video) ? new_image_blue : {red,green,blue};
//assign {red_out, green_out, blue_out} = (mode & ~sop & packet_video) ? new_image_yellow : {red,green,blue};
//assign {red_out, green_out, blue_out} = (mode & ~sop & packet_video) ? new_image_white : {red,green,blue};

//Count valid pixels to get the image coordinates. Reset and detect packet type on Start of Packet.
reg [10:0] x, y;
reg packet_video;
always@(posedge clk) begin
	if (sop) begin
		x <= 11'h0;
		y <= 11'h0;
		packet_video <= (blue[3:0] == 3'h0);
	end
	else if (in_valid) begin
		if (x == IMAGE_W-1) begin
			x <= 11'h0;
			y <= y + 11'h1;
		end
		else begin
			x <= x + 11'h1;
		end
	end
end

//Find first and last red pixels
reg [10:0] x_min_red, y_min_red, x_max_red, y_max_red;
reg [10:0] x_min_blue, y_min_blue, x_max_blue, y_max_blue;
reg [10:0] x_min_yellow, y_min_yellow, x_max_yellow, y_max_yellow;
reg [10:0] x_min_white, y_min_white, x_max_white, y_max_white;

always@(posedge clk) begin
	if (in_valid) begin
			// Update bounds for red color
			if (found_red) begin
				if (x < x_min_red) x_min_red <= x;
				if (x > x_max_red) x_max_red <= x;
				if (y < y_min_red) y_min_red <= y;
				y_max_red <= y;
			end
			// Update bounds for blue color
			if (found_blue) begin
				if (x < x_min_blue) x_min_blue <= x;
				if (x > x_max_blue) x_max_blue <= x;
				if (y < y_min_blue) y_min_blue <= y;
				y_max_blue <= y;
			end
			// Update bounds for yellow color
			if (found_yellow) begin
				if (x < x_min_yellow) x_min_yellow <= x;
				if (x > x_max_yellow) x_max_yellow <= x;
				if (y < y_min_yellow) y_min_yellow <= y;
				y_max_yellow <= y;
			end
			// Update bounds for white color
			if (found_white) begin
				if (x < x_min_white) x_min_white <= x;
				if (x > x_max_white) x_max_white <= x;
				if (y < y_min_white) y_min_white <= y;
				y_max_white <= y;
			end
	end
	if (sop & in_valid) begin
		// Reset bounds for red color
		x_min_red <= 11'h7ff;
		y_min_red <= 10'h3ff;
		x_max_red <= 0;
		y_max_red <= 0;
		// Reset bounds for blue color
		x_min_blue <= 11'h7ff;
		y_min_blue <= 10'h3ff;
		x_max_blue <= 0;
		y_max_blue <= 0;
		// Reset bounds for yellow color
		x_min_yellow <= 11'h7ff;
		y_min_yellow <= 10'h3ff;
		x_max_yellow <= 0;
		y_max_yellow <= 0;
		// Reset bounds for white color
		x_min_white <= 11'h7ff;
		y_min_white <= 10'h3ff;
		x_max_white <= 0;
		y_max_white <= 0;
	end
end

//Process bounding box at the end of the frame.
reg [3:0] msg_state;

reg [7:0] frame_count;

always@(posedge clk) begin
	if (eop & in_valid & packet_video) begin  //Ignore non-video packets
		
		//Latch edges for display overlay on next frame for red
		left_red <= x_min_red;
		right_red <= x_max_red;
		top_red <= y_min_red;
		bottom_red <= y_max_red;

		//Latch edges for display overlay on next frame for blue
		left_blue <= x_min_blue;
		right_blue <= x_max_blue;
		top_blue <= y_min_blue;
		bottom_blue <= y_max_blue;

		//Latch edges for display overlay on next frame for yellow
		left_yellow <= x_min_yellow;
		right_yellow <= x_max_yellow;
		top_yellow <= y_min_yellow;
		bottom_yellow <= y_max_yellow;

		//Latch edges for display overlay on next frame for white
		left_white <= x_min_white;
		right_white <= x_max_white;
		top_white <= y_min_white;
		bottom_white <= y_max_white;
		
		
		//Start message writer FSM once every MSG_INTERVAL frames, if there is room in the FIFO
		frame_count <= frame_count - 1;
		
		if (frame_count == 0 && msg_buf_size < MESSAGE_BUF_MAX - 3) begin
			msg_state <= 4'b0001;
			frame_count <= MSG_INTERVAL-1;
		end
	end
	
	//Cycle through message writer states once started
	if (msg_state != 4'b0000) msg_state <= msg_state + 4'b0001;

end
	
//Generate output messages for CPU
reg [31:0] msg_buf_in; 
wire [31:0] msg_buf_out;
reg msg_buf_wr;
wire msg_buf_rd, msg_buf_flush;
wire [7:0] msg_buf_size;
wire msg_buf_empty;

`define RED_BOX_MSG_ID "RBB"

always@(*) begin	//Write words to FIFO as state machine advances
	case(msg_state)
		4'b0000: begin
			msg_buf_in = 32'b0;
			msg_buf_wr = 1'b0;
		end
		4'b0001: begin
			msg_buf_in = {5'b0, x_min_red, 5'b0, y_min_red}; // Top Left
			msg_buf_wr = 1'b1;
		end
		4'b0010: begin
			msg_buf_in = {5'b0, x_max_red, 5'b0, y_max_red}; // Bottom Right
			msg_buf_wr = 1'b1;
		end
		4'b0011: begin
			msg_buf_in = {5'b0, x_min_blue, 5'b0, y_min_blue}; 
			msg_buf_wr = 1'b1;
		end
		4'b0100: begin
			msg_buf_in = {5'b0, x_max_blue, 5'b0, y_max_blue}; 
			msg_buf_wr = 1'b1;
		end
		4'b0101: begin
			msg_buf_in = {5'b0, x_min_yellow, 5'b0, y_min_yellow}; 
			msg_buf_wr = 1'b1;
		end
		4'b0110: begin
			msg_buf_in = {5'b0, x_max_yellow, 5'b0, y_max_yellow}; 
			msg_buf_wr = 1'b1;
		end
		4'b0111: begin
			msg_buf_in = {5'b0, x_min_white, 5'b0, y_min_white}; 
			msg_buf_wr = 1'b1;
		end
		4'b1000: begin
			msg_buf_in = {5'b0, x_max_white, 5'b0, y_max_white}; 
			msg_buf_wr = 1'b1;
		end
	endcase
end


//Output message FIFO
MSG_FIFO	MSG_FIFO_inst (
	.clock (clk),
	.data (msg_buf_in),
	.rdreq (msg_buf_rd),
	.sclr (~reset_n | msg_buf_flush),
	.wrreq (msg_buf_wr),
	.q (msg_buf_out),
	.usedw (msg_buf_size),
	.empty (msg_buf_empty)
	);


//Streaming registers to buffer video signal
STREAM_REG #(.DATA_WIDTH(26)) in_reg (
	.clk(clk),
	.rst_n(reset_n),
	.ready_out(sink_ready),
	.valid_out(in_valid),
	.data_out({red,green,blue,sop,eop}),
	.ready_in(out_ready),
	.valid_in(sink_valid),
	.data_in({sink_data,sink_sop,sink_eop})
);

STREAM_REG #(.DATA_WIDTH(26)) out_reg (
	.clk(clk),
	.rst_n(reset_n),
	.ready_out(out_ready),
	.valid_out(source_valid),
	.data_out({source_data,source_sop,source_eop}),
	.ready_in(source_ready),
	.valid_in(in_valid),
	.data_in({red_out, green_out, blue_out, sop, eop})
);


/////////////////////////////////
/// Memory-mapped port		 /////
/////////////////////////////////

// Addresses
`define REG_STATUS    			0
`define READ_MSG    				1
`define READ_ID    				2
`define REG_BBCOL					3

//Status register bits
// 31:16 - unimplemented
// 15:8 - number of words in message buffer (read only)
// 7:5 - unused
// 4 - flush message buffer (write only - read as 0)
// 3:0 - unused


// Process write

reg  [7:0]   reg_status;
reg	[23:0]	bb_col;

always @ (posedge clk)
begin
	if (~reset_n)
	begin
		reg_status <= 8'b0;
		bb_col <= BB_COL_DEFAULT;
	end
	else begin
		if(s_chipselect & s_write) begin
		   if      (s_address == `REG_STATUS)	reg_status <= s_writedata[7:0];
		   if      (s_address == `REG_BBCOL)	bb_col <= s_writedata[23:0];
		end
	end
end


//Flush the message buffer if 1 is written to status register bit 4
assign msg_buf_flush = (s_chipselect & s_write & (s_address == `REG_STATUS) & s_writedata[4]);


// Process reads
reg read_d; //Store the read signal for correct updating of the message buffer

// Copy the requested word to the output port when there is a read.
always @ (posedge clk)
begin
   if (~reset_n) begin
	   s_readdata <= {32'b0};
		read_d <= 1'b0;
	end
	
	else if (s_chipselect & s_read) begin
		if   (s_address == `REG_STATUS) s_readdata <= {16'b0,msg_buf_size,reg_status};
		if   (s_address == `READ_MSG) s_readdata <= {msg_buf_out};
		if   (s_address == `READ_ID) s_readdata <= 32'h1234EEE2;
		if   (s_address == `REG_BBCOL) s_readdata <= {8'h0, bb_col};
	end
	
	read_d <= s_read;
end

//Fetch next word from message buffer after read from READ_MSG
assign msg_buf_rd = s_chipselect & s_read & ~read_d & ~msg_buf_empty & (s_address == `READ_MSG);
						


endmodule
