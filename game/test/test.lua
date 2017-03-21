BOOTING = "BOOTING"
SETUP_COMPLETE = "SETUP COMPLETE"
WAITING_FOR_MENU = "WAITING FOR MENU"
WAITING_FOR_LEVEL2_LOAD = "WAITING FOR LEVEL 2 LOAD"
WAITING_FOR_RECORD_START = "WAITING FOR RECORD START"
WAITING_FOR_LEVEL2_END =  "WAITING FOR LEVEL 2 END"
CHECK_LEVEL2_SCORE = "CHECK LEVEL2 SCORE"
SUCCESS = "SUCCESS"

state = BOOTING

function Setup()
      uae_write_symbol16("_script_port", 0)
      uae_log("CreateLogFile")
      log = io.open("lua.log", "w")
      io.output(log)
      uae_warp()
end

function Quit()
	 uae_quit()
end

stateMachine = {
      [BOOTING] = { next = WAITING_FOR_MENU, callback = Setup },
      [WAITING_FOR_MENU] = { next = WAITING_FOR_LEVEL2_LOAD, wait = {"_menu_mode", 1}, write = {"_script_port", 2} },
      [WAITING_FOR_LEVEL2_LOAD] = { next = WAITING_FOR_RECORD_START, wait = {"_menu_mode", 0}, write = {"_script_port", 1}},
      [WAITING_FOR_RECORD_START] = { next = WAITING_FOR_LEVEL2_END, wait = {"_record_state", 2}},
      [WAITING_FOR_LEVEL2_END] = { next = CHECK_LEVEL2_SCORE, wait = {"_record_state", 0}},
      [CHECK_LEVEL2_SCORE] = { next = SUCCESS, check = {"_game_score", 13461, 32}, callback = Quit},
      [SUCCESS] = { }
}

quit = false

function Tick()

	if not quit then 
	local transition = false

	if stateMachine[state].next then
	   	if stateMachine[state].check then
		   if stateMachine[state].check[3] == 32 then
		      if uae_peek_symbol32(stateMachine[state].check[1]) == stateMachine[state].check[2] then
		      	 transition = true
		      else
			 quit = true
                      end
		   elseif (uae_peek_symbol16(stateMachine[state].check[1]) == stateMachine[state].check[2]) then
	      	      transition = true
		   else
		      quit = true
	           end
		else
	    
		if stateMachine[state].wait then
		   if stateMachine[state].wait[3] == 32 and (uae_peek_symbol32(stateMachine[state].wait[1]) == stateMachine[state].wait[2]) then
		      transition = true
		   end		   
	   	   if (uae_peek_symbol16(stateMachine[state].wait[1]) == stateMachine[state].wait[2]) then
	      	      transition = true
	           end
		else
		   transition = true
		end
		end

		if transition then
	   	   if stateMachine[state].callback then
	      	      stateMachine[state].callback()
	   	   end
		   if stateMachine[state].write then
		      io.write("writing ", stateMachine[state].write[2], " to ", stateMachine[state].write[1], "\n")
		      uae_write_symbol16(stateMachine[state].write[1], stateMachine[state].write[2])
		   end
	   	   io.write("state transition ", state, " -> ", stateMachine[state].next, "\n");
	   	   state = stateMachine[state].next
	        end
	
	end

	io.flush()

	if quit then
	   Quit()
	end
	end

end

function on_uae_vsync()
	 Tick()
end
