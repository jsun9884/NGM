"use strict";

const jwt         = require('jsonwebtoken');
const config      = require('config');

exports.authenticate = function(req, res, next){
    let profile = config.users.filter(function(user){
        return user.email == req.body.username && user.password == req.body.password;
    });

    if(profile.length){
        let token = jwt.sign(profile[0], config.JWT_SECRET.secret, { expiresIn: '1440m'});
        res.json({
            token: token,
            profile: {
                'first_name' : profile[0]['first_name'],
                'last_name' : profile[0]['last_name'],
                'role' : profile[0]['role']
            }
        });
    }
    else {
        res.status(401).send('Authentication Error.Invalid Credentials');
    }
};