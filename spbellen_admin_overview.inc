<?php

function spbellen_overview() {
  $stats = spbellen_get_all_stats();
  $variables = array(
    'campaign_stats' => $stats
  );
  return theme('spbellen_overview_theme', $variables);
}
