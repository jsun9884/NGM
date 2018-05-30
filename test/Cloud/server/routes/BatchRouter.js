'use strict';

const router = require('express').Router();
const batchController = require('../controllers/batchController');

router.route('/batch/command')
    .post(batchController.batchSendCommand);

module.exports = router;