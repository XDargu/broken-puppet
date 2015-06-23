all_co={}

function startCoroutine( name, func, ... )
	local co = coroutine.create( func )
	coroutine.resume( co, ... )
	all_co[ name ] = co
end

function clearCoroutines( )
	local name, co = next( all_co )
	while name ~= nil do
		all_co[ name ] = nil
		name, co = next( all_co, name )
	end
	print( "Coroutines cleared.")
end

function dumpCoroutines( )
	local name, co = next( all_co )
	while name ~= nil do
		local co_status = coroutine.status( co )
		print( "COROUTINE: " .. name .. " [" .. tostring(co_status) .. "]")
		
		name, co = next( all_co, name )
	end
end

function updateCoroutines( elapsed )
	local name, co = next( all_co )
	while name ~= nil do
		local co_status = coroutine.status( co )
		if co_status == "dead" then
			all_co[ name ] = nil
		else
			coroutine.resume( co, elapsed )
		end
		name, co = next( all_co, name )
	end
end

function waitTime( t )
	local waiting_time = 0
	while waiting_time < t do
		waiting_time = waiting_time + coroutine.yield( )
	end
end

function waitCondition( func, ... )
	while true do
		if func( ... ) then
			return true
		end
		coroutine.yield( )
	end
end

print( "Coroutines registered")