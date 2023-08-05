import React, { useState, useEffect} from 'react'
import ReactPolling from 'react-polling/lib/ReactPolling'
import GridMap from './gridmap' // Import the Gridmap component
import * as Matrix from './shortestPath'
import './App.css'

function App() {
   const httpAddress = `http://localhost:3001/`

   //React States
   const [polledData, updatePollData] = useState('')
   const [redPosition, setRedPosition] = useState({ x: 0, y: 0 }) // new state variable to store current 
   const [bluePosition, setBluePosition] = useState({ x: 0, y: 0 }) // new state variable to store current
   const [wallFlag, setWallFlag] = useState(false)    //false - left wall, true - right wall
   const [positions, setPositions] = useState([{}])   // new state variable to store all previous grid positions
   const [orientation, setOrientation] = useState(0)  // new state variable to store current orientation
   const [gridMatrix, setGridMatrix] = useState(null)
   const [dotMode, setDotMode] = useState(0)      //0 = StartMode 1 = RoverMode, 2 = DotMode - for shortest path
   const [redOrBlueDot, setRedOrBlueDot] = useState(false) //false = Red, true = Blue
   const [startPosition, setStartPosition] = useState({ x: 0, y: 0 })   //Initial Position of rover
   const [lastGridPosition, setLastGridPosition] = useState({ x: 0, y: 0 }) //Last grid position of rover

   //Grid size variables
   let boxHeight = 500
   let boxWidth = 500
   let colSize = 60
   let rowSize = boxHeight/boxWidth * colSize
   let cellWidth = boxWidth/colSize
   let cellHeight = boxHeight/rowSize

  //polling stuff
   const fetchData = () => {
      return fetch(`${httpAddress}pollServer/`)
   }

   const pollingSuccess = (jsonResponse) => {      //polling success check
      const txtres = ''
      updatePollData(txtres)
      return true
   }

   const pollingFailure = () => {                  //polling failure check 
      console.log('Polling failed')
      return true
   }


   //Send dotMode to backend
   const sendDotMode  = (dotMode) => {
      fetch(`${httpAddress}DotMode/`, {
         method: 'POST',
         mode: 'cors',
         headers: {
         'Content-Type': 'application/json',
         },
         body: JSON.stringify({ dotMode: dotMode }), // Convert wallFlag to JSON string
      })
      .then((res) => res.status)
      .catch((err) => console.log("DotMode: " + err))
   }

   //sql data test
   useEffect(() => {                         
      ///See CORS
      fetch(`${httpAddress}personQuery/`)
      .then((res) => res.json())
      .then((data) => console.log(JSON.stringify(data)))
      .catch((err) => console.log("personQuery: " + err))
   }, [])

   //Start Routine
   const startRoutine = () => {
      if (dotMode === 0) {
         setStartPosition ({
            x: Math.floor(redPosition.x / cellWidth) * cellWidth,
            y: Math.floor(redPosition.y / cellHeight) * cellHeight
         })
         fetch(`${httpAddress}RoverStart/`, {
            method: 'POST',
            mode: 'cors',
            headers: {
               'Content-Type': 'application/json',
            },
            body: JSON.stringify({ start: "true" }), 
         })
         .then((res) => res.status)
         .catch((err) => console.log("Rover Start: " + err))
         setDotMode(1)
         sendDotMode(1)
      }
   }
   
   // Arrow input move dot
   const handleArrowKeys = (event) => { 
      event.preventDefault()
      if (redOrBlueDot === false) {
         switch (event.key) {
         case 'ArrowUp':
            setRedPosition((prevRedPosition) => ({ ...prevRedPosition, y: prevRedPosition.y - cellHeight }))
            break
         case 'ArrowDown':
            setRedPosition((prevRedPosition) => ({ ...prevRedPosition, y: prevRedPosition.y + cellHeight }))
            break
         case 'ArrowLeft':
            setRedPosition((prevRedPosition) => ({ ...prevRedPosition, x: prevRedPosition.x - cellWidth }))
            break
         case 'ArrowRight':
            setRedPosition((prevRedPosition) => ({ ...prevRedPosition, x: prevRedPosition.x + cellWidth }))
         default:
            break
         }
      } else if (redOrBlueDot === true) {
         switch (event.key) {
         case 'ArrowUp':
            setBluePosition((prevBluePosition) => ({ ...prevBluePosition, y: prevBluePosition.y - cellHeight }))
            break
         case 'ArrowDown':
            setBluePosition((prevBluePosition) => ({ ...prevBluePosition, y: prevBluePosition.y + cellHeight }))
            break
         case 'ArrowLeft':
            setBluePosition((prevBluePosition) => ({ ...prevBluePosition, x: prevBluePosition.x - cellWidth }))
            break
         case 'ArrowRight':
            setBluePosition((prevBluePosition) => ({ ...prevBluePosition, x: prevBluePosition.x + cellWidth }))
         default:
            break
         }
      }
   }

   //Effect to move dot on arrow input
   useEffect(() => {  
      document.addEventListener('keydown', handleArrowKeys)
      return () => {
      document.removeEventListener('keydown', handleArrowKeys)
      }
   }, [redOrBlueDot])


   //Function to store position
   const storePosition = (x, y) => {
      if (dotMode === 1) {
         const xGrid = Math.floor(x/cellWidth)
         const yGrid = Math.floor(y/cellHeight)
         const newPosition = {x: xGrid, y: yGrid}
         // Check if the position already exists
         if (!positions.some(pos => pos.x === xGrid && pos.y === yGrid)) {
            // console.log("Position Stored: " + newPosition.x + ", " + newPosition.y)
            setPositions(prevRedPositions => [...prevRedPositions, newPosition])  // Append the new position to the existing array
         } else if ((newPosition.x !== lastGridPosition.x) || (newPosition.y !== lastGridPosition.y)){
            setWallFlag(!wallFlag)
            console.log("Wall Flag: " + wallFlag)
         }
         setLastGridPosition(newPosition) // Set the last grid position to the new position
      }
   }

   //Animation for dot
   const dotAnimation =  () => {
      const dot = redOrBlueDot ? document.getElementById("startDot") : document.getElementById("roverDot") 
      dot.style.width = "20px"  // Increase the size of the dot
      dot.style.height = "20px" 
      setTimeout(() => { // Set a timeout to revert the size of the dot after half a second
         dot.style.width = `${cellWidth * 5/8}px`
         dot.style.height = `${cellHeight * 5/8}px`
      }, 100) 
   }

   // Function to move dot to where mouse clicks
   const handleMouseDown = (event) => {
      const box = document.getElementById("box")
      const rect = box.getBoundingClientRect()
      const x = event.clientX - rect.left
      const y = event.clientY - rect.top
      if (x >= 0 && x <= boxWidth && y >= 0 && y <= boxHeight) {
         if (redOrBlueDot === false) {
            setRedPosition({ x: x, y: y })
         } else if (redOrBlueDot === true) {
            setBluePosition({ x: x, y: y })
         }
         dotAnimation()
      }
      document.addEventListener('mousemove', handleMouseMove)
      document.addEventListener('mouseup', handleMouseUp)
   } 

   // Function to move dot to where mouse is dragged
   const handleMouseMove = (event) => {
      const box = document.getElementById("box")
      const rect = box.getBoundingClientRect()
      const x = event.clientX - rect.left
      const y = event.clientY - rect.top
      if (x >= 0 && x <= boxWidth && y >= 0 && y <= boxHeight) {
         if (redOrBlueDot === false) {
            setRedPosition({ x: x, y: y })
         } else if (redOrBlueDot === true) {
            setBluePosition({ x: x, y: y })
         }
      }
   } 

   // Function to remove mousemove event listener
   const handleMouseUp = () => {
      document.removeEventListener('mousemove', handleMouseMove)
      document.removeEventListener('mouseup', handleMouseUp)
   }

   //Effect to move dot on mouse click and drag
   useEffect(() => {
      document.addEventListener('mousedown', handleMouseDown)
      return () => {
         document.removeEventListener('mousedown', handleMouseDown)
      }
   }, [redOrBlueDot])

   //Effect to Store position
   useEffect(() => {
      storePosition(redPosition.x, redPosition.y)
   }, [redPosition.x, redPosition.y])  

   //Update dot position from HTTP
   const updatePosition = () => {
      fetch(`${httpAddress}RoverData/`)
      .then((res) => res.json())
      .then((data) =>{
         let x_val = parseInt(data.x)
         let y_val = parseInt(data.y)
         if (data.x === undefined || data.y === undefined){ 
            setRedPosition((prevRedPosition) => ({ ...prevRedPosition}))  //set position to previous position
         }
         else{
            console.log("new position: " + data.x + " " + data.y)
            setRedPosition({x: x_val + startPosition.x, y: y_val + startPosition.y})   //set position to new position relative to startPosition
            setOrientation(data.orientation)
         }
      })
      .catch((err) => console.log("RoverData: " + err))
   }
   
   //Effect to call Update Postiion
   useEffect(() => {    
      let interval 
      if (dotMode === 1) {
         interval = setInterval(() => {
            updatePosition()
         }, 1)
      }
      return () => clearInterval(interval) 
   },[dotMode])

   //Effect to send wallFlag
   useEffect(() => {
      let leftOrRight = ""
      if (wallFlag) {
         leftOrRight = JSON.stringify({ flag: "right" })
      } else {
         leftOrRight = JSON.stringify({ flag: "left" })
      }
      fetch(`${httpAddress}RoverControl/`, {
        method: 'POST',
        mode: 'cors',
        headers: {
          'Content-Type': 'application/json',
        },
        body: leftOrRight, // Convert wallFlag to JSON string
      })
      .then((res) => res.status)
      .catch((err) => console.log("Rover Controls: ", err)) 
    }, [wallFlag]) 

   //Function of Orientation Button
   const handleOrientation = () => {
      if (orientation < 45) {
      setOrientation(50) 
      } else {
      setOrientation(40) 
      }
   } 

   //Plots shortest path
   const plotPath  = () =>  {
      let endRow = Math.floor(redPosition.y/cellHeight) + 1
      let endCol = Math.floor(redPosition.x/cellWidth) + 1
      let startRow = Math.floor(bluePosition.y/cellHeight) + 1
      let startCol = Math.floor(bluePosition.x/cellWidth) + 1
      // console.log("startRow: " + startRow + " startCol: " + startCol + " endRow: " + endRow + " endCol: " + endCol)
      const cells = document.querySelectorAll(".cell") 
      cells.forEach((cell) => {
         const isGreen = cell.style.backgroundColor === "turquoise"
         if (isGreen) {
            cell.style.backgroundColor = "white"


         }
      }) 
      if (gridMatrix) {
         let shortestPath =  Matrix.findShortestPath(gridMatrix, startRow, startCol, endRow, endCol) 
         shortestPath.forEach(({ row, col }) => {
            const cell = document.querySelector(`.cell[data-row="${row}"][data-col="${col}"]`)
            cell.style.backgroundColor = "turquoise"
         }) 
      } else {
         alert("Maze mapping not complete.\nPlease click FINISH if maze has been mapped") 
      }
   }

   //Routine once maze mapping is complete
   const finishRoutine  = () => {
      if (dotMode === 2){
         alert("Maze already mapped")
      } else if (dotMode === 1) {
         alert("Maze mapping complete")
         const newGridMatrix = Matrix.createPath(cellHeight, cellHeight, rowSize, colSize)
         setGridMatrix(newGridMatrix)
         setDotMode(2)
         sendDotMode(2)
      }
   }
   
   //Handle Enter Key
   useEffect(() => {
      if (dotMode === 0) {
         const handleKeyEnter = (event) => {
            if (event.key === "Enter") {
               startRoutine()
            }
         }
         document.addEventListener("keydown", handleKeyEnter)
         return () => document.removeEventListener("keydown", handleKeyEnter)
      }
      else if (dotMode === 1) {
         const handleKeyEnter = (event) => {
            if (event.key === "Enter") {
               finishRoutine()
            }
         }
         document.addEventListener("keydown", handleKeyEnter)
         return () => document.removeEventListener("keydown", handleKeyEnter)
      } else if (dotMode === 2) {
         const handleKeyEnter = (event) => {
            if (event.key === "Enter") {
               plotPath()
            }
         } 
         document.addEventListener("keydown", handleKeyEnter)
         return () => document.removeEventListener("keydown", handleKeyEnter)
      }

   }, [gridMatrix, redPosition, bluePosition, dotMode]) 

   //Handle Tab Key
   useEffect(() => {
      const handleTabKey = (event) => {
         if (event.key === "Tab") {
            setRedOrBlueDot(!redOrBlueDot) 
         }
      } 
      document.addEventListener("keydown", handleTabKey) 
      return () => document.removeEventListener("keydown", handleTabKey) 
   }, [redOrBlueDot]) 

   return (
      <div className="App" style={{ backgroundImage: `url(${require('./mazeimage.jpg')})` }}>
         
      <ReactPolling
         url={`${httpAddress}pollServer/`}
         interval={500}
         retryCount={3}
         onSuccess={pollingSuccess}
         onFailure={pollingFailure}
         promise={fetchData}
         render={({ startPolling, stopPolling, isPolling }) => {
            return (
            <div>
               {polledData}
               <br />
               <br />
            </div>
            )
         }}
      />

      <GridMap 
         redPosition = {redPosition}
         bluePosition = {bluePosition}
         boxHeight = {boxHeight} 
         boxWidth = {boxWidth}
         colSize = {colSize}
         rowSize = {rowSize} 
         cellHeight = {cellHeight}
         cellWidth = {cellWidth}
         wallFlag = {wallFlag}
         orientation = {orientation}
         dotMode = {dotMode}
         redOrBlueDot = {redOrBlueDot}
      />
      
      {/* Manual Testing Buttons */}

     <div style={{ position: "absolute", top: "40%", left: "15%", transform: "translate(-50%, -50%)" }}>
        <button 
            style={{ 
               flex: 1, 
               height: "50px", 
               width: "200px", 
               borderRadius: "25px",
               backgroundColor: "grey",
               border: "none",
               color: "white",
               textAlign: "center",
               textDecoration: "none",
               display: "inline-block",
               fontSize: "16px",
               margin: "4px 2px",
               cursor: "pointer"
            }} 
               onClick={() => {
               startRoutine()
            }}>
               START
            </button>
         
        </div>

        <div style={{ position: "absolute", top: "60%", left: "15%", transform: "translate(-50%, -50%)" }}>
            <button 
               style={{ 
                  flex: 1, 
                  height: "50px", 
                  width: "200px", 
                  borderRadius: "25px",
                  backgroundColor: "grey",
                  border: "none",
                  color: "white",
                  textAlign: "center",
                  textDecoration: "none",
                  display: "inline-block",
                  fontSize: "16px",
                  margin: "4px 2px",
                  cursor: "pointer"
               }} 
               onClick={() => {
                  finishRoutine()
               }}
            >
               FINISH
            </button>
        </div>

      <div style={{ position: "absolute", top: "20%", right: "5%", transform: "translate(-50%, -50%)",  }}>
         <button 
            style={{ 
               flex: 1, 
               height: "50px", 
               width: "200px", 
               borderRadius: "25px",
               backgroundColor: "grey",
               border: "none",
               color: "white",
               textAlign: "center",
               textDecoration: "none",
               display: "inline-block",
               fontSize: "16px",
               margin: "4px 2px",
               cursor: "pointer"
            }} 
            onClick={() => setWallFlag(!wallFlag)}
         >
            Change Left or Right
         </button>
      </div>

      <div style={{ position: "absolute", top: "80%", right: "5%", transform: "translate(-50%, -50%)" }}>
         <button
            style={{ 
               flex: 1, 
               height: "50px", 
               width: "200px", 
               borderRadius: "25px",
               backgroundColor: "grey",
               border: "none",
               color: "white",
               textAlign: "center",
               textDecoration: "none",
               display: "inline-block",
               fontSize: "16px",
               margin: "4px 2px",
               cursor: "pointer"
            }} 
            onClick={handleOrientation}
         >
            Change Orientation
         </button>
      </div>

      <div style={{ position: "absolute", top: "40%", right: "5%", transform: "translate(-50%, -50%)" }}>
      <button             
         style={{ 
            flex: 1, 
            height: "50px", 
            width: "200px", 
            borderRadius: "25px",
            backgroundColor: "grey",
            border: "none",
            color: "white",
            textAlign: "center",
            textDecoration: "none",
            display: "inline-block",
            fontSize: "16px",
            margin: "4px 2px",
            cursor: "pointer"
         }}  onClick={() => {
            setRedOrBlueDot(!redOrBlueDot)
         }}>
            Start or End Dot
         </button>
      </div>

      <div style={{ position: "absolute", top: "60%", right: "5%", transform: "translate(-50%, -50%)" }}>
         <button 
            style={{ 
               flex: 1, 
               height: "50px", 
               width: "200px", 
               borderRadius: "25px",
               backgroundColor: "grey",
               border: "none",
               color: "white",
               textAlign: "center",
               textDecoration: "none",
               display: "inline-block",
               fontSize: "16px",
               margin: "4px 2px",
               cursor: "pointer"
            }} 
            onClick={() => {
               plotPath()
            }}
         >
            Shortest Path
         </button>
      
      </div>

        
      </div>
   )
}

export default App
