import React, { useState, useEffect } from 'react'

function GridMap(props) {
  
  //Props passed down
  let redPosition = props.redPosition
  let bluePosition = props.bluePosition
  let boxHeight= props.boxHeight
  let boxWidth = props.boxWidth
  let colSize = props.colSize
  let rowSize = props.rowSize
  let wallFlag = props.wallFlag
  let cellHeight = props.cellHeight
  let cellWidth = props.cellWidth
  let orientation = props.orientation
  let dotMode = props.dotMode
  let redOrBlueDot = props.redOrBlueDot
 
  const containerStyle = {
    display: 'flex',
    justifyContent: 'center',
    alignItems: 'center',
    height: '100vh'
  } 

  const boxStyle = {
    width: boxWidth + 'px',
    height: boxHeight + 'px',
    backgroundColor: 'transparent',
    border: '1px solid black',
    borderRadius: '0px',
    display: 'grid',
    gridTemplateColumns: `repeat(${colSize}, 1fr)`,
    gridTemplateRows: `repeat(${rowSize}, 1fr)`,
    gap: '0px',
    position: 'relative', 
    overflow: 'hidden',
    marginBottom: '20px'
  } 

  const roverDotStyle = {
    position: 'absolute',
    borderRadius: '5px',
    backgroundColor: 'red',
    left: 
       Math.min(
         Math.max(Math.floor(redPosition.x / cellWidth) * cellWidth + cellWidth * 0.1625, 0), 
         boxWidth - 5
       ) + 'px',
    top: 
      Math.min(
        Math.max(Math.floor(redPosition.y / cellWidth) * cellWidth + cellWidth * 0.1625, 0), 
        boxHeight - 5
      ) + 'px',
    width: `${cellWidth * 5/8}px`,
    height: `${cellHeight * 5/8}px`,
   }
 
  const startDotStyle = {
    position: 'absolute',
    borderRadius: '5px',
    backgroundColor:  dotMode === 2 ? 'blue' : 'transparent',
    left: 
    Math.min(
      Math.max(Math.floor(bluePosition.x / cellWidth) * cellWidth + cellWidth * 0.1625, 0), 
      boxWidth - 5
    ) + 'px',
    top:
    Math.min(
      Math.max(Math.floor(bluePosition.y / cellHeight) * cellHeight + cellHeight * 0.1625, 0), 
      boxHeight - 5
    ) + 'px',

    width: `${cellWidth * 5/8}px`,
    height: `${cellHeight * 5/8}px`,
  }


  //Create Initial Grid Cells
  const cells = [] 
  for (let row = 0; row < rowSize; row++) {
    for (let col = 0; col < colSize; col++) {
      const index = row * rowSize + col 
      const cellStyle = {
        backgroundColor: "black",
        border: "0.1px solid white"
      } 
      cells.push(
        <div key={index} className="cell" data-row={row} data-col={col} style={cellStyle}></div>
      ) 
    }
  }

  //Update Cells next to dot
  const updateAdjacentCells = (row,col,updateWidth,mode) => {
    if (dotMode === 1) {
      for (let i = 1; i <= updateWidth; i++) {
        let rowOffset
        let colOffset
        if (mode == "row"){
          rowOffset = wallFlag ? i : -i
          colOffset = 0
        } else if (mode == "col"){
          rowOffset = 0
          colOffset = wallFlag ? i : -i
        }
        const adjacentCell = document.querySelector(`[data-row="${row+rowOffset}"][data-col="${col+colOffset}"]`) 
        if (adjacentCell) {
          adjacentCell.style.backgroundColor = "white" 
        }
      } 
    }
  }

  //Update Grid Function
  const updateGrid = () => {
    if (dotMode === 1) {
      const cells = document.querySelectorAll(".cell") 
      let updateWidth = 1
      cells.forEach((cell) => {
        const row = parseInt(cell.dataset.row) 
        const col = parseInt(cell.dataset.col) 
        if (row === Math.floor(redPosition.y / cellHeight) && col === Math.floor(redPosition.x / cellWidth)) {
          cell.style.backgroundColor = "white" 
          if (orientation < 45) {
            updateAdjacentCells(row,col,updateWidth,"col")
          } else if (orientation >= 45) {
            updateAdjacentCells(row,col,updateWidth,"row")
          }
        } 
      }) 
    }
  }

  //Effect to call update Grid
  useEffect(() => {
    updateGrid()
  }, [redPosition, wallFlag]) 

  return (
    <div style={containerStyle}>
      <div id="box" style={boxStyle}>
        {cells}
        <div id="roverDot" style={roverDotStyle}/>
        <div id="startDot" style={startDotStyle}/>
      </div>
    </div>
  ) 
}

export default GridMap 