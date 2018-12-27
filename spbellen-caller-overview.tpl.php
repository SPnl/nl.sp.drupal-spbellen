<?php
/*
 * Available variables:
 *   campaign_stats
 *     title
 *     total
 *     blacklisted
 *     call_back
 *     completed
 *     wrong_number
 */
?>

<?php if(!empty($general_stats_caller)) : ?>
  <h3><?php print($general_stats_caller_campaign['campaign_title']); ?></h3>
  <h3><?php print($general_stats_caller['caller_display_name']); ?></h3>
  <p><br/></p>
  <h3>Algemene statistieken</h3>
  <?php foreach ($general_stats_caller as $caller_id => $data) : ?>
    <?php if (is_numeric($caller_id)) : ?>
      <table style="width: auto">
        <tr>
          <th></th>
          <th colspan="3">beller&nbsp;totaal</th>
          <th colspan="3">beller&nbsp;campagne</th>
          <th colspan="3">campagne</th>
          <?php if (!empty($general_stats_caller_campaign_date)) : ?>
            <th colspan="3">beller&nbsp;campagne<br/><?php print($general_stats_caller_campaign_date['call_date']); ?></th>
            <th colspan="3">campagne<br/><?php print($general_stats_campaign_date['call_date']); ?></th>
          <?php endif; ?>
        </tr>
        <tr>
          <td width="200">Afgerond</td>
          <td class="numeric" width="1"><?php print $general_stats_caller[$caller_id]['answered_completed']; ?></td>
          <td class="numeric" width="1"><?php print $general_stats_caller[$caller_id]['answered_completed_percentage']; ?>%</td>
          <td></td>
          <td class="numeric" width="1"><?php print $general_stats_caller_campaign[$caller_id]['answered_completed']; ?></td>
          <td class="numeric" width="1"><?php print $general_stats_caller_campaign[$caller_id]['answered_completed_percentage']; ?>%</td>
          <td></td>
          <td class="numeric" width="1"><?php print $general_stats_campaign['answered_completed']; ?></td>
          <td class="numeric" width="1"><?php print $general_stats_campaign['answered_completed_percentage']; ?>%</td>
          <td></td>
          <?php if (!empty($general_stats_caller_campaign_date)) : ?>
            <td class="numeric" width="1"><?php print $general_stats_caller_campaign_date[$caller_id]['answered_completed']; ?></td>
            <td class="numeric" width="1"><?php print $general_stats_caller_campaign_date[$caller_id]['answered_completed_percentage']; ?>%</td>
            <td></td>
            <td class="numeric" width="1"><?php print $general_stats_campaign_date['answered_completed']; ?></td>
            <td class="numeric" width="1"><?php print $general_stats_campaign_date['answered_completed_percentage']; ?>%</td>
            <td></td>
          <?php endif; ?>
        </tr>
        <tr>
          <td>Terugbellen</td>
          <td class="numeric"><?php print $general_stats_caller[$caller_id]['answered_call_back']; ?></td>
          <td class="numeric"><?php print $general_stats_caller[$caller_id]['answered_call_back_percentage']; ?>%</td>
          <td></td>
          <td class="numeric"><?php print $general_stats_caller_campaign[$caller_id]['answered_call_back']; ?></td>
          <td class="numeric"><?php print $general_stats_caller_campaign[$caller_id]['answered_call_back_percentage']; ?>%</td>
          <td></td>
          <td class="numeric"><?php print $general_stats_campaign['answered_call_back']; ?></td>
          <td class="numeric"><?php print $general_stats_campaign['answered_call_back_percentage']; ?>%</td>
          <td></td>
          <?php if (!empty($general_stats_caller_campaign_date)) : ?>
            <td class="numeric" width="1"><?php print $general_stats_caller_campaign_date[$caller_id]['answered_call_back']; ?></td>
            <td class="numeric" width="1"><?php print $general_stats_caller_campaign_date[$caller_id]['answered_call_back_percentage']; ?>%</td>
            <td></td>
            <td class="numeric" width="1"><?php print $general_stats_campaign_date['answered_call_back']; ?></td>
            <td class="numeric" width="1"><?php print $general_stats_campaign_date['answered_call_back_percentage']; ?>%</td>
            <td></td>
          <?php endif; ?>
        </tr>
        <tr>
        <tr>
          <td>Belafspraak</td>
          <td class="numeric"><?php print $general_stats_caller[$caller_id]['call_appointment']; ?></td>
          <td class="numeric"><?php print $general_stats_caller[$caller_id]['call_appointment_percentage']; ?>%</td>
          <td></td>
          <td class="numeric"><?php print $general_stats_caller_campaign[$caller_id]['call_appointment']; ?></td>
          <td class="numeric"><?php print $general_stats_caller_campaign[$caller_id]['call_appointment_percentage']; ?>%</td>
          <td></td>
          <td class="numeric"><?php print $general_stats_campaign['call_appointment']; ?></td>
          <td class="numeric"><?php print $general_stats_campaign['call_appointment_percentage']; ?>%</td>
          <td></td>
          <?php if (!empty($general_stats_caller_campaign_date)) : ?>
            <td class="numeric" width="1"><?php print $general_stats_caller_campaign_date[$caller_id]['call_appointment']; ?></td>
            <td class="numeric" width="1"><?php print $general_stats_caller_campaign_date[$caller_id]['call_appointment_percentage']; ?>%</td>
            <td></td>
            <td class="numeric" width="1"><?php print $general_stats_campaign_date['call_appointment']; ?></td>
            <td class="numeric" width="1"><?php print $general_stats_campaign_date['call_appointment_percentage']; ?>%</td>
            <td></td>
          <?php endif; ?>
        </tr>
        <tr>
          <td>Overgeslagen</td>
          <td class="numeric"><?php print $general_stats_caller[$caller_id]['skipped']; ?></td>
          <td class="numeric"><?php print $general_stats_caller[$caller_id]['skipped_percentage']; ?>%</td>
          <td></td>
          <td class="numeric"><?php print $general_stats_caller_campaign[$caller_id]['skipped']; ?></td>
          <td class="numeric"><?php print $general_stats_caller_campaign[$caller_id]['skipped_percentage']; ?>%</td>
          <td></td>
          <td class="numeric"><?php print $general_stats_campaign['skipped']; ?></td>
          <td class="numeric"><?php print $general_stats_campaign['skipped_percentage']; ?>%</td>
          <td></td>
          <?php if (!empty($general_stats_caller_campaign_date)) : ?>
            <td class="numeric" width="1"><?php print $general_stats_caller_campaign_date[$caller_id]['skipped']; ?></td>
            <td class="numeric" width="1"><?php print $general_stats_caller_campaign_date[$caller_id]['skipped_percentage']; ?>%</td>
            <td></td>
            <td class="numeric" width="1"><?php print $general_stats_campaign_date['skipped']; ?></td>
            <td class="numeric" width="1"><?php print $general_stats_campaign_date['skipped_percentage']; ?>%</td>
            <td></td>
          <?php endif; ?>
        <tr>
          <td>Verkeerd nummer</td>
          <td class="numeric"><?php print $general_stats_caller[$caller_id]['wrong_number']; ?></td>
          <td class="numeric"><?php print $general_stats_caller[$caller_id]['wrong_number_percentage']; ?>%</td>
          <td></td>
          <td class="numeric"><?php print $general_stats_caller_campaign[$caller_id]['wrong_number']; ?></td>
          <td class="numeric"><?php print $general_stats_caller_campaign[$caller_id]['wrong_number_percentage']; ?>%</td>
          <td></td>
          <td class="numeric"><?php print $general_stats_campaign['wrong_number']; ?></td>
          <td class="numeric"><?php print $general_stats_campaign['wrong_number_percentage']; ?>%</td>
          <td></td>
          <?php if (!empty($general_stats_caller_campaign_date)) : ?>
            <td class="numeric" width="1"><?php print $general_stats_caller_campaign_date[$caller_id]['wrong_number']; ?></td>
            <td class="numeric" width="1"><?php print $general_stats_caller_campaign_date[$caller_id]['wrong_number_percentage']; ?>%</td>
            <td></td>
            <td class="numeric" width="1"><?php print $general_stats_campaign_date['wrong_number']; ?></td>
            <td class="numeric" width="1"><?php print $general_stats_campaign_date['wrong_number_percentage']; ?>%</td>
            <td></td>
          <?php endif; ?>
        </tr>
        </tr>
        <tr>
          <td>Nooit meer bellen</td>
          <td class="numeric"><?php print $general_stats_caller[$caller_id]['blacklisted']; ?></td>
          <td class="numeric"><?php print $general_stats_caller[$caller_id]['blacklisted_percentage']; ?>%</td>
          <td></td>
          <td class="numeric"><?php print $general_stats_caller_campaign[$caller_id]['blacklisted']; ?></td>
          <td class="numeric"><?php print $general_stats_caller_campaign[$caller_id]['blacklisted_percentage']; ?>%</td>
          <td></td>
          <td class="numeric"><?php print $general_stats_campaign['blacklisted']; ?></td>
          <td class="numeric"><?php print $general_stats_campaign['blacklisted_percentage']; ?>%</td>
          <td></td>
          <?php if (!empty($general_stats_caller_campaign_date)) : ?>
            <td class="numeric" width="1"><?php print $general_stats_caller_campaign_date[$caller_id]['blacklisted']; ?></td>
            <td class="numeric" width="1"><?php print $general_stats_caller_campaign_date[$caller_id]['blacklisted_percentage']; ?>%</td>
            <td></td>
            <td class="numeric" width="1"><?php print $general_stats_campaign_date['blacklisted']; ?></td>
            <td class="numeric" width="1"><?php print $general_stats_campaign_date['blacklisted_percentage']; ?>%</td>
            <td></td>
          <?php endif; ?>
        </tr>
        <tr>
          <td>Totaal (exclusief niet opgenomen)</td>
          <td class="numeric"><?php print $general_stats_caller[$caller_id]['total']; ?></td>
          <td class="numeric">100%</td>
          <td></td>
          <td class="numeric"><?php print $general_stats_caller_campaign[$caller_id]['total']; ?></td>
          <td class="numeric"><?php print $general_stats_caller_campaign[$caller_id]['total_percentage']; ?>%</td>
          <td></td>
          <td class="numeric"><?php print $general_stats_campaign['total']; ?></td>
          <td class="numeric"><?php print $general_stats_campaign['total_percentage']; ?>%</td>
          <td></td>
          <?php if (!empty($general_stats_caller_campaign_date)) : ?>
            <td class="numeric" width="1"><?php print $general_stats_caller_campaign_date[$caller_id]['total']; ?></td>
            <td class="numeric" width="1"><?php print $general_stats_caller_campaign_date[$caller_id]['total_percentage']; ?>%</td>
            <td></td>
            <td class="numeric" width="1"><?php print $general_stats_campaign_date['total']; ?></td>
            <td class="numeric" width="1"><?php print $general_stats_campaign_date['total_percentage']; ?>%</td>
            <td></td>
          <?php endif; ?>
        </tr>
        <tr>
          <td width="200">Niet opgenomen</td>
          <td class="numeric" width="1"><?php print $general_stats_caller[$caller_id]['not_answered']; ?></td>
          <td class="numeric" width="1"><?php print $general_stats_caller[$caller_id]['not_answered_percentage']; ?>%</td>
          <td></td>
          <td class="numeric" width="1"><?php print $general_stats_caller_campaign[$caller_id]['not_answered']; ?></td>
          <td class="numeric" width="1"><?php print $general_stats_caller_campaign[$caller_id]['not_answered_percentage']; ?>%</td>
          <td></td>
          <td class="numeric" width="1"><?php print $general_stats_campaign['not_answered']; ?></td>
          <td class="numeric" width="1"><?php print $general_stats_campaign['not_answered_percentage']; ?>%</td>
          <td></td>
          <?php if (!empty($general_stats_caller_campaign_date)) : ?>
            <td class="numeric" width="1"><?php print $general_stats_caller_campaign_date[$caller_id]['not_answered']; ?></td>
            <td class="numeric" width="1"><?php print $general_stats_caller_campaign_date[$caller_id]['not_answered_percentage']; ?>%</td>
            <td></td>
            <td class="numeric" width="1"><?php print $general_stats_campaign_date['not_answered']; ?></td>
            <td class="numeric" width="1"><?php print $general_stats_campaign_date['not_answered_percentage']; ?>%</td>
            <td></td>
          <?php endif; ?>
        </tr>
      </table>
    <?php endif; ?>
  <?php endforeach; ?>
<?php endif; ?>

<?php if (!empty($result_stats_caller_campaign)) : ?>
  <p><br/></p>
  <h3>Resultaat statistieken</h3>
  <?php foreach ($result_stats_caller_campaign['components'] as $cid => $data) : ?>
    <p><strong><?php print($data['name']); ?></strong></p>
    <table style="width: auto">
      <tr>
        <th></th>
        <th colspan="2">beller</th>
        <th colspan="2">campagne</th>
        <?php if (!empty($result_stats_caller_campaign_date)) : ?>
          <th colspan="2">beller<br/><?php print $result_stats_caller_campaign_date['call_date']; ?></th>
          <th colspan="2">campagne<br/><?php print $result_stats_caller_campaign_date['call_date']; ?></th>
        <?php endif; ?>
      </tr>
      <?php foreach ($data['options'] as $key => $option) : ?>
        <tr>
          <td width="200"><?php print($option); ?></td>
          <td class="numeric" width="1"><?php print($data['stats'][$key]); ?></td>
          <td class="numeric"><?php print($data['stats'][$key. '_percentage']); ?>%</td>
          <td class="numeric" width="1"><?php print($result_stats_campaign['components'][$cid]['stats'][$key]); ?></td>
          <td class="numeric"><?php print($result_stats_campaign['components'][$cid]['stats'][$key . '_percentage']); ?>%</td>
          <?php if (!empty($result_stats_caller_campaign_date)) : ?>
            <td class="numeric" width="1"><?php print($result_stats_caller_campaign_date['components'][$cid]['stats'][$key]); ?></td>
            <td class="numeric"><?php print($result_stats_caller_campaign_date['components'][$cid]['stats'][$key . '_percentage']); ?>%</td>
            <td class="numeric" width="1"><?php print($result_stats_campaign_date['components'][$cid]['stats'][$key]); ?></td>
            <td class="numeric"><?php print($result_stats_campaign_date['components'][$cid]['stats'][$key . '_percentage']); ?>%</td>
          <?php endif; ?>
        </tr>
      <?php endforeach; ?>
    </table>
  <?php endforeach; ?>
<?php endif; ?>
