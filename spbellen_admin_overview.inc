<?php

function spbellen_overview() {
  $stats = spbellen_get_all_stats();
  $variables = array(
    'campaign_stats' => $stats['campaign_stats'],
    'general_stats' => $stats['general_stats'],
  );
  return theme('spbellen_overview_theme', $variables);
}
