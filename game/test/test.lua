state = "BOOTING"
quit = false


function Setup()
   uae_write_symbol16("_script_port", 0)
   log = io.open("lua.log", "w")
   io.output(log)
   uae_warp()
end


function Quit()
   uae_quit()
end


stateMachine = {
   ["BOOTING"] = {
      next = "WAITING FOR MENU",
      callback = Setup
   },
   ["WAITING FOR MENU"] = {
      wait = {"_menu_mode", 1},
      next = "WAITING FOR LEVEL 2 LOAD",
      write = {"_script_port", string.byte('2')}
   },
   ["WAITING FOR LEVEL 2 LOAD"] = {
      wait = {"_menu_mode", 0},
      next = "WAITING FOR RECORD START",
      write = {"_script_port", string.byte('P')}
   },
   ["WAITING FOR RECORD START"] = {
      wait = {"_record_state", 2},
      next =  "WAITING FOR LEVEL 2 END"
   },
   ["WAITING FOR LEVEL 2 END"] = {
      wait = {"_record_state", 0},
      next = "VERIFY LEVEL 2 PARAMETERS"
   },
   ["VERIFY LEVEL 2 PARAMETERS"] = {
      less = {{"_game_total", 769000, 32}},
      equal = {{"_game_score", 13461, 32}, {"_game_lives", 2, 32}},
      next = "PASSED",
      callback = Quit
   },
   ["PASSED"] = { }
}


function Tick()
   if not quit then 
      local transition = false
      local asserts = false
      
      if stateMachine[state].next then
	 if stateMachine[state].equal then
	    asserts = true
	    for i, equal in ipairs(stateMachine[state].equal) do
	       if equal[3] == 32 then
		  if uae_peek_symbol32(equal[1]) == equal[2] then
		  else
		     io.write("FAILED: ", equal[1], " (", uae_peek_symbol32(equal[1]), ") != ", equal[2], "\n");
		     quit = true
		  end
	       elseif (uae_peek_symbol16(equal[1]) == equal[2]) then
	       else
		  io.write("FAILED: ", equal[1], " (",uae_peek_symbol16(equal[1]), ") != ", equal[2], "\n");
		  quit = true
	       end
	    end
	 end
	 
	 if stateMachine[state].less then
	    asserts = true
	    for i, less in ipairs(stateMachine[state].less) do
	       if less[3] == 32 then
		  if uae_peek_symbol32(less[1]) < less[2] then
		  else
		     io.write("FAILED: ", less[1], " (",uae_peek_symbol32(less[1]), ") >= ", less[2], "\n");
		     quit = true
		  end
	       elseif (uae_peek_symbol16(less[1]) < less[2]) then
	       else
		  io.write("FAILED: ", less[1], " (",uae_peek_symbol16(less[1]), ") >= ", less[2], "\n");
		  quit = true
	       end
	    end
	 end
	 
	 if asserts == false then
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
	 else
	    if quit == false then
	       transition = true
	    end
	 end
	 
	 if transition then
	    if stateMachine[state].callback then
	       stateMachine[state].callback()
	    end
	    if stateMachine[state].write then
	       io.write("wrote ", stateMachine[state].write[2], " to ", stateMachine[state].write[1], "\n")
	       uae_write_symbol16(stateMachine[state].write[1], stateMachine[state].write[2])
	    end
	    io.write(state, " -> ", stateMachine[state].next, "\n");
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
