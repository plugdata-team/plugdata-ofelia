local log = ofLog("ofMouseEntered")
local canvas = ofCanvas(this)
local order, enabled, simplify = 50, true, true

function M.new()
  local args = canvas:getArgs()
  if #args == 1 then
    order = args[1]
  elseif #args == 2 then
    order, enabled = args[1], args[2] ~= 0
  elseif #args == 3 then
    order, enabled, simplify = args[1], args[2] ~= 0, args[3] ~= 0
  elseif #args ~= 0 then
    log:error("requires 1, 2 or 3 creation arguments")
  end
  ofWindow.addListener("mouseEntered", this, order)
end

function M.setOrder(f)
  order = f
  ofWindow.addListener("mouseEntered", this, order)
end

function M.setEnabled(b)
  enabled = b ~= 0
end

function M.setSimplify(b)
  simplify = b ~= 0
end

function M.free()
  ofWindow.removeListener("mouseEntered", this)
end

function M.mouseEntered(e)
  if not enabled then
    return
  end
  if not simplify then
    return {e.type, e.x, e.y, e.button, e.scrollX, e.scrollY, e.modifiers}
  end
  return {e.x, e.y}
end