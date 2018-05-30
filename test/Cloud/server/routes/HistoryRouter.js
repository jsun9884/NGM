'use strict';

const router = require('express').Router();
const historyController = require('../controllers/historyController');

router.route('/history/events/recent')
    .get(historyController.getRecentEvents);

router.route('/history/events/:deviceId')
    .get(historyController.getEvents);

router.route('/history/commands/:deviceId')
    .get(historyController.getCommands);

// Export Events data
router.route('/history/exports/events/:deviceId')
    .get(historyController.exportEvents);

// Export Regular data
router.route('/history/exports/regular/:deviceId')
    .get(historyController.exportRegularData);

module.exports = router;