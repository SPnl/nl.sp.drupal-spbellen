<?php

function spbellen_overview() {
  $stats = spbellen_get_all_stats();
  if (empty($stats['campaign_stats'])) {
    return '<p>Er zijn nog geen campaignes aangemaakt.</p>';
  }
  else {
    $variables = array(
      'campaign_stats' => isset($stats['campaign_stats']) ? $stats['campaign_stats'] : array(),
      'general_stats' => isset($stats['general_stats']) ? $stats['general_stats'] : array(),
    );
    return theme('spbellen_overview_theme', $variables);
  }
}
