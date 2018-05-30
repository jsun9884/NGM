'use strict';

var multer = require('multer');

const UPLOAD_PATH = "/tmp";
const MAX_FILE_SIZE_BYTES = 10 * 1024 * 1024;

//var upload = multer({
//    dest: UPLOAD_PATH,
//    limits: { fileSize: MAX_FILE_SIZE_BYTES, files: 1 }
//});

exports.fileUploadHandler = function (req, res, next) {
    if(!!req.files.file){
        req.files.file.mv('/tmp/' + req.files.file.name, function(err) {
            err ? next(err) : res.json({ status : true});

            //if (err)
            //    return res.status(500).send(err);

            //res.send('File uploaded!');
        });
    }
    else{
        next(new Error("Wrong specified file name"));
    }

    //upload(req, res, function (err) {
    //    err ? next(err) : res.json.bind({ status : true})
    //})

    // console.log(req.file); //form files
    /* example output:
     { fieldname: 'upl',
     originalname: 'grumpy.png',
     encoding: '7bit',
     mimetype: 'image/png',
     destination: './uploads/',
     filename: '436ec561793aa4dc475a88e84776b1b9',
     path: 'uploads/436ec561793aa4dc475a88e84776b1b9',
     size: 277056 }
     */

};
