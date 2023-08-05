//Create matrix from grid
export function createPath (cellHeight, cellWidth, rowSize, colSize) {
  const cells = document.querySelectorAll(".cell") 
  const numRows = Math.min(Math.ceil(window.innerHeight / cellHeight), rowSize) 
  const numCols = Math.min(Math.ceil(window.innerWidth / cellWidth), colSize) 
  const matrix = Array(numRows).fill().map(() => Array(numCols).fill(0)) 
  cells.forEach((cell) => {
      const row = parseInt(cell.dataset.row) 
      const col = parseInt(cell.dataset.col) 
      if (row < numRows && col < numCols) {
          const isWhite = cell.style.backgroundColor === "white" 
          matrix[row][col] = isWhite ? 1 : 0 
      }
  }) 

  return matrix
}



export function findShortestPath(matrix, startRow, startCol, endRow, endCol) {
  const numRows = matrix.length 
  const numCols = matrix[0].length 
  const distances = Array(numRows).fill().map(() => Array(numCols).fill(Infinity)) 
  const directions = [
    { row: -1, col: 0 },  // Up
    { row: 1, col: 0 },   // Down
    { row: 0, col: -1 },  // Left
    { row: 0, col: 1 },    // Right
    //diagonal
    // { row: -1, col: -1 },   // Up-Left
    // { row: -1, col: 1 },    // Up-Right
    // { row: 1, col: -1 },    // Down-Left
    // { row: 1, col: 1 },     // Down-Right
  ] 

  // Initialize the starting cell
  distances[startRow - 1][startCol - 1] = 0 

  // Queue to store cells to visit
  const queue = [{ row: startRow - 1, col: startCol - 1, path: [{ row: startRow - 1, col: startCol - 1 }] }] 

  // Variable to store the shortest path
  let shortestPath = [] 

  while (queue.length > 0) {
    const { row, col, path } = queue.shift() 

    // Explore neighboring cells
    for (const dir of directions) {
      const newRow = row + dir.row 
      const newCol = col + dir.col 

      // Check if the neighboring cell is within bounds and is a white cell
      if (
        newRow >= 0 &&
        newRow < numRows &&
        newCol >= 0 &&
        newCol < numCols &&
        matrix[newRow][newCol] === 1
      ) {
        // Calculate the new distance
        const newDistance = distances[row][col] + 1 

        // Check if the new distance is shorter
        if (newDistance < distances[newRow][newCol]) {
          distances[newRow][newCol] = newDistance 

          // Create a new path by extending the current path
          const newPath = [...path, { row: newRow, col: newCol }] 

          // If the new cell is the end cell, update the shortest path
          if (newRow === endRow - 1 && newCol === endCol - 1) {
            shortestPath = newPath 
          }

          // Enqueue the neighboring cell with the new path
          queue.push({ row: newRow, col: newCol, path: newPath }) 
        }
      }
    }
  }
  return shortestPath
}








