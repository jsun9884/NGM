'use strict';

const router              = require('express').Router();
const loginController     = require('../controllers/loginController');

router.route('/login')
    .post(loginController.authenticate);

module.exports = router;