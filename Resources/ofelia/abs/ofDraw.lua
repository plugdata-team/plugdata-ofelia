local log = ofLog("ofDraw")
local outlet = ofOutlet(this)
local order, enabled, styleAuto, matrixAuto = 50, true, true, true

function M.new()
  ofWindow.addListener("draw", this, order)
end

function M.setOrder(f)
  order = f
  ofWindow.addListener("draw", this, order)
end

function M.setEnabled(b)
  enabled = b ~= 0
end

function M.setStyleAuto(b)
  styleAuto = b ~= 0
end

function M.setMatrixAuto(b)
  matrixAuto = b ~= 0
end

function M.setAuto(b)
  M.setStyleAuto(b)
  M.setMatrixAuto(b)
end

function M.free()
  ofWindow.removeListener("draw", this)
end

function M.draw()
  if not enabled then
    return
  end
  if styleAuto then
    ofPushStyle()
  end
  if matrixAuto then
    ofPushMatrix()
  end
  outlet:outletBang(0)
  if matrixAuto then
    ofPopMatrix()
  end
  if styleAuto then
    ofPopStyle()
  end
end