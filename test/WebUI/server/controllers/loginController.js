"use strict";

var jwt         = require('jsonwebtoken'),
    config      = require('config'),
    users       = require('../config/users.json');

exports.authenticate = function(req, res, next){
    var profile = users.filter(function(user){
        return user.email == req.body.username && user.password == req.body.password;
    });

    if(profile.length){

        // TODO Change Expire Time
        var token = jwt.sign(profile[0], config.JWT_SECRET.secret, { expiresIn: '1m'});
        res.json({
            token: token,
            profile: {
                'first_name' : profile[0]['first_name'],
                'last_name' : profile[0]['last_name'],
                'role' : profile[0]['role']
            }
        });
    }
    else{
        res.send(401, 'Authentication Error.Invalid Credentials');
    }
};