//set up the basic dependencies required to create an Express.js server with CORS support and path utilities
const express = require("express") //imports the Express.js module, which allows you to create and configure web servers in Node.js
const cors = require('cors') //imports the 'cors' module, which is a middleware for Express that enables Cross-Origin Resource Sharing. CORS allows requests from different origins to access your server's resources
const path = require('path') //imports the built-in Node.js module 'path'. The 'path' module provides utilities for working with file and directory paths
const PORT = process.env.PORT || 3001 
const app = express() //creates an instance of the Express application. The app object will be used to configure the server and define the server's behavior
const htmlParser = require('node-html-parser')
const { type } = require("os")

app.use(express. json());
app.use(express.urlencoded ({ extended: false }));

app.use(cors({ //server will accept requests from any domain
 origin: '*'
}))
app.use(express.static(path.resolve(__dirname, './client/build'))) //serves static files using the Express.js static middleware. It specifies the directory where the static files are located
app.use(cors({ 
 methods: ['GET','POST','DELETE','UPDATE','PUT','PATCH',] //methods property is set to an array containing the HTTP methods that are allowed for CORS requests. By specifying these methods, you control which types of requests are accepted by your server
}))

app.route('/')
  .get((req, res) => {
    console.log(req.body)
    res.sendStatus(200)
  })
  .post((req, res) => {
    console.log(req.body)
    res.sendStatus(200)
  })

app.route("/RoverStart")
  .get((req,res) => {
    console.log(startFlag)
    if (startFlag){
        console.log("Sending Start")
        res.send("Start").status(200)
    } else {
        console.log("Sending Wait")
        res.send("Wait").status(200)
    }
  })
  .post((req,res) => {
    startFlag = req.body.start
    console.log("startFlag: " + startFlag)
    res.sendStatus(200)
  })

const scalePosition = (currentPosition) => {
    let scale = 50
    let scaledPosition = currentPosition
    scaledPosition.x = scaledPosition.x * scale
    scaledPosition.y = scaledPosition.y * scale
    let rawQuotient = scaledPosition.orientation/360
    let remainder = rawQuotient % 1
    let quotient = rawQuotient - remainder
    scaledPosition.orientation = scaledPosition.orientation - 360 * quotient
    if (scaledPosition.orientation < 0) {
        scaledPosition.orientation = -scaledPosition.orientation
        scaledPosition.orientation = 360 - scaledPosition.orientation
    }
    return scaledPosition
}


//Rover Data Endpoint
let pos_count = 0
const position = [{}]
app.route("/RoverData")
    .get((req, res) => {                    //Send Positions to Frontend
        if(pos_count >= position.length){
            res.json("no more positions")
            return
        } else if ( position[pos_count].x == undefined || position[pos_count].y == undefined) {
            res.json("no position recieved")
        } else{
            if (dotMode.dotMode == 1) {
                res.send(position[pos_count])
            }
        }
        pos_count++
    })
    .post((req, res) => {                   //Recieve Position from Rover
        let scaledPosition = scalePosition(req.body)
        if (dotMode.dotMode == 1){
            position.push(scaledPosition)
        }
        console.log(req.body)
        res.send(position).status(200)
    })

//Rover Control Endpoint - Handles Wall-Flag
let wallFlag = 'left'
app.route("/RoverControl")
    .get((req, res) => {            //Send Wall-Flag to Rover
        console.log("wallFlag: " + wallFlag)
        res.send(wallFlag).status(200)
    })
    .post((req, res) => {           //Receive Wall-Flag from Frontend
        wallFlag = req.body.flag
        res.sendStatus(200)
    })

let dotMode = 0
app.route("/DotMode")
    .get((req,res) => {

    })
    .post((req,res) => {
        dotMode = req.body
        res.sendStatus(200)
        console.log(dotMode)
    })



// app.get("/personQuery", (req, res) => { //same as function(req,res)
//     con.query("INSERT INTO Persons VALUES(1, 'ADAM', 'PAUL', 'UCL', 'CONGO')") //only adds onto database, returns nothing
//     con.query("SELECT * FROM Persons", function (err, result, fields) { //same as (err, result, fields) =>
//     if (err) throw err //if error in query then throws an error
//     res.json(result) // converts query result to json so it is ready to transmit
//     })
// })
    
app.get("/pollServer", (req, res) => {
    var d = new Date()
    const json_res = {
    "time" : d.toTimeString() // json format
    }
    res.send(json_res) // no need to convert to json so direct send
}) 

app.get('*', (req, res) => { // undoes CORS to allow any host to access resources
    res.sendFile(path.resolve(__dirname)) // standard
})

app.listen(PORT, () => { // same as function()
 console.log(`Server listening on ${PORT}`) //$ concatenates string
}) 

// var mysql = require('mysql') //imports external SQL module
// var con = mysql.createConnection( //creates connection in order to interact with MySQL database
// {
//  host: "13.53.94.205", //public ec2 instance IP address
//  user: "username",
//  password: "usrpwd",
//  database: "Persondb" //name of database
// })
// con.connect(function(err) { //attempts to establish a connection to the MySQL database
//  if (err) throw err //If an error occurs, it will be thrown and stop the execution of the code
//  console.log("Successfully connected to the database...\n") //If the connection is successful, the success message will be logged
// })

