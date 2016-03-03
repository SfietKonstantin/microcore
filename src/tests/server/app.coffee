express = require 'express'
morgan = require 'morgan'
bodyParser = require 'body-parser'
http = require 'http'

app = express()
app.set 'port', 8080
app.use morgan('dev')
app.use bodyParser.urlencoded({extended: 'true'})
app.use bodyParser.json()

# routes
app.get '/api/get', (req, res) ->
    res.send 'Hello world from /api/get'

app.get '/api/geterror', (req, res) ->
    res.status(401).send 'Error from /api/geterror'

app.post '/api/post', (req, res) ->
    res.send 'Hello world from /api/post'

app.post '/api/posterror', (req, res) ->
    res.status(401).send 'Error from /api/posterror'

app.delete '/api/delete', (req, res) ->
    res.send 'Hello world from /api/delete'

app.delete '/api/deleteerror', (req, res) ->
    res.status(401).send 'Error from /api/deleteerror'

server = http.createServer(app).listen app.get('port')
console.log("Server started on port #{app.get('port')}")
