test = 1
state = "startup"
quit = false
screenShotFilename = "out/test-screenshot.png"

function Setup()
   uae_write_symbol16("_script_port", 0)
   log = io.open("lua.log", "w")
   io.output(log)
   uae_warp()
   io.write("\n\n=== test started ===\n")
end


function Quit()
   quit = true
   io.flush()
   log:close()
   uae_quit()
end


function Reset()
   uae_reset()
end


function Write(symbol, value)
   io.write("ram write: ", symbol, " = ", value, "\n")
   uae_write_symbol16(symbol, value)
end


function CheckLevel2Screenshot(filename)
   local screenshot1 = io.open(screenShotFilename, "rb")
   local screenshot2 = io.open(filename, "rb")
   if screenshot1:read("*all") ~= screenshot2:read("*all") then
      io.write("FAIL: ", screenShotFilename, " != ", filename, "\n")
      io.flush()
      quit = true
   else
      io.write("PASS: ", screenShotFilename, " == ", filename, "\n")
   end
   screenshot1:close()
   screenshot2:close()   
end


function Screenshot(_state)
   if screenShotState == 0 or screenShotState == nil then
      screenShotState = 1
      Write("_script_port",  _state.screenShotFrame + 0x8000)
   elseif screenShotState == 1 then
      if uae_peek_symbol32("_game_paused") == 1 then
	 screenShotState = 2
	 screenShotFrame = uae_peek_symbol32("_hw_verticalBlankCount")
      end
   elseif screenShotState == 2 then
      if uae_peek_symbol32("_hw_verticalBlankCount") > screenShotFrame+500 then
	 screenShotState = 3
	 screenShotFrame = uae_peek_symbol32("_hw_verticalBlankCount")
	 uae_screenshot(screenShotFilename)
      end
   elseif screenShotState == 3 then
      if uae_peek_symbol32("_hw_verticalBlankCount") > screenShotFrame+500 then
	 CheckLevel2Screenshot(_state.filename)
	 Write("_script_port", string.byte(' '))
	 screenShotState = 4
      end
   elseif screenShotState == 4 then
      if uae_peek_symbol32("_game_paused") == 0 then
	 screenShotState = 0
	 return true
      end
   end
   return false
end


setup = {
   ["startup"] = {
      next = "startup complete",
      exitState = Setup
   },
   ["startup complete"] = {}
}


level2 = {
   ["booting"] = {
      next = "waiting for menu",
      wait = {"_menu_mode", 0},      
   },
   ["waiting for menu"] = {
      wait = {"_menu_mode", 1},
      next = "waiting for level to load",
      write = {"_script_port", string.byte('2')}
   },
   ["waiting for level to load"] = {
      wait = {"_menu_mode", 0},
      next = "waiting for record start",
      write = {"_script_port", string.byte('P')}
   },
   ["waiting for record start"] = {
      wait = {"_record_state", 2},
      next =  "screenshot1",
   },
   ["screenshot1"] = {
      screenShotFrame = 2000,
      filename = "test/screenshot.png",
      transition = Screenshot,
      next = "screenshot2"
   },
   ["screenshot2"] = {
      screenShotFrame = 3000,
      filename = "test/screenshot2.png",
      transition = Screenshot,
      next = "waiting for level end",
   },
   ["waiting for level end"] = {
      wait = {"_record_state", 0},
      next = "verify level parameters",
   },
   ["verify level parameters"] = {
      less = {{"_game_total", 765648, 32}},
      equal = {{"_game_score", 13461, 32}, {"_game_lives", 2, 32}},
      next = "back to menu"
   },
   ["back to menu"] = {
      write = {"_script_port", string.byte('Q')},
      next = "done"
   },
   ["done"] = {}      
}


reset = {
   ["booting"] = {
      next = "done",
      exitState = Reset
   },
   ["done"] = {}
}


tests = {
   { setup, "setup" },
   { level2, "level 2 : first pass"},
   { reset, "reset" },
   { level2, "level 2 : second pass"}
}


function Tick(stateMachine)
   if not quit then 
      local transition = false
      local asserts = false
      
      if stateMachine[state].next then
	 if stateMachine[state].equal then
	    asserts = true
	    for i, equal in ipairs(stateMachine[state].equal) do
	       if equal[3] == 32 then
		  local value = uae_peek_symbol32(equal[1])
		  if  value == equal[2] then
		     io.write("PASS: ", equal[1], " ", value, " == ", equal[2], "\n")
		  else
		     io.write("FAIL: ", equal[1], " ", value, " != ", equal[2], "\n")
		     quit = true
		  end
	       else
		  local value = uae_peek_symbol16(equal[1])
		  if (value == equal[2]) then
		     io.write("PASS: ", equal[1], " ", value, " == ", equal[2], "\n")
		  else
		     io.write("FAIL: ", equal[1], " ", value, " != ", equal[2], "\n")
		     quit = true
		  end
	       end
	    end
	 end
	 
	 if not quit and stateMachine[state].less then
	    asserts = true
	    for i, less in ipairs(stateMachine[state].less) do
	       if less[3] == 32 then
		  if uae_peek_symbol32(less[1]) < less[2] then
		     io.write("PASS: ", less[1], " (",uae_peek_symbol32(less[1]), ") < ", less[2], "\n")
		  else
		     io.write("FAIL: ", less[1], " (",uae_peek_symbol32(less[1]), ") >= ", less[2], "\n")
		     quit = true
		  end
	       elseif (uae_peek_symbol16(less[1]) < less[2]) then
	       else
		  io.write("FAIL: ", less[1], " (",uae_peek_symbol16(less[1]), ") >= ", less[2], "\n")
		  quit = true
	       end
	    end
	 end
	 
	 if not quit and asserts == false then
	    if stateMachine[state].wait then
	       if stateMachine[state].wait[3] == 32 then
		  if (uae_peek_symbol32(stateMachine[state].wait[1]) == stateMachine[state].wait[2]) then
		     io.write("trigger: ", stateMachine[state].wait[1], " == ", stateMachine[state].wait[2], "\n")
		     transition = true
		  end
	       else
		  if (uae_peek_symbol16(stateMachine[state].wait[1]) == stateMachine[state].wait[2]) then
		     io.write("trigger: ", stateMachine[state].wait[1], " == ", stateMachine[state].wait[2], "\n")
		     transition = true
		  end
	       end
		  
	    else
	       transition = true
	    end
	 else
	    if not quit then
	       transition = true
	    end
	 end

	 
	 if not quit and stateMachine[state].transition then
	    transition = stateMachine[state].transition(stateMachine[state])
	 end
	 
	 if transition then
	    if stateMachine[state].exitState then
	       stateMachine[state].exitState()
	    end

	    if not quit then 
	       if  stateMachine[state].write then
		  Write(stateMachine[state].write[1], stateMachine[state].write[2])
	       end
	       io.write("state: ", state, " -> ", stateMachine[state].next, "\n")
	       state = stateMachine[state].next
	       if stateMachine[state].enterState then
		  stateMachine[state].enterState(stateMachine[state])
	       end
	    end
	 end	 
      else
	 test = test + 1
	 if tests[test] then
	    io.write("=== next test: ", tests[test][2], " ===\nstate: ", state, " -> ", "booting\n")
	 end

	 state = "booting"
      end
      
      io.flush()
      
      if quit then
	 Quit()
      end
   end   
end


function on_uae_vsync()
   if tests[test] then 
      Tick(tests[test][1])
   elseif not quit then
      io.write("ALL TESTS PASSED!\n")
      quit = true
      Quit()
   end
end
